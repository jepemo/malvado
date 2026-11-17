#!/usr/bin/env python3
"""Reject incomplete, unrelated and stale candidate workflow evidence."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import platform_candidate as candidate

SHA = 'a' * 40
RUN = dict(id=123, run_attempt=1, html_url='https://github.com/jepemo/malvado/actions/runs/123',
           head_sha=SHA, path='.github/workflows/ci.yml', event='push',
           status='completed', conclusion='success',
           repository={'full_name': 'jepemo/malvado'},
           head_repository={'full_name': 'jepemo/malvado'})


class CandidateTests(unittest.TestCase):
    def test_reject_missing_skipped_failed_or_running_jobs(self):
        for jobs in ([], [{'status': 'completed', 'conclusion': 'skipped'}],
                     [{'status': 'completed', 'conclusion': 'failure'}],
                     [{'status': 'in_progress', 'conclusion': None}]):
            with self.subTest(jobs=jobs), self.assertRaises(ValueError):
                candidate.validate_jobs([{'jobs': jobs}])

    def test_success(self):
        for workflow in candidate.WORKFLOWS.values():
            run = dict(RUN, path='.github/workflows/' + workflow)
            self.assertEqual(candidate.validate_run(run, SHA, workflow, 'jepemo/malvado')['sha'], SHA)

    def test_reject_unrelated_or_incomplete_evidence(self):
        changes = dict(head_sha='b' * 40, path='.github/workflows/other.yml',
                       event='pull_request', status='in_progress', conclusion='cancelled',
                       repository={'full_name': 'other/repo'},
                       head_repository={'full_name': 'fork/repo'})
        for field, value in changes.items():
            with self.subTest(field=field), self.assertRaises(ValueError):
                candidate.validate_run(dict(RUN, **{field: value}), SHA, 'ci.yml', 'jepemo/malvado')
        for field in RUN:
            if field in ('id', 'html_url', 'run_attempt'):
                continue
            run = copy.deepcopy(RUN)
            del run[field]
            with self.subTest(missing=field), self.assertRaises(ValueError):
                candidate.validate_run(run, SHA, 'ci.yml', 'jepemo/malvado')

    def test_abbreviated_sha_rejected(self):
        with self.assertRaises(ValueError):
            candidate.validate_run(RUN, SHA[:7], 'ci.yml', 'jepemo/malvado')

    def test_failed_query_removes_old_success(self):
        with tempfile.TemporaryDirectory() as folder:
            output = Path(folder) / 'result.json'
            output.write_text('old success')
            argv = ['candidate', '--sha', SHA, '--ci', '123', '--windows', '124',
                    '--android', '125', '--output', str(output)]
            with patch.object(sys, 'argv', argv), patch.object(candidate.subprocess, 'run', side_effect=subprocess.TimeoutExpired('gh', 60)):
                with self.assertRaises(subprocess.TimeoutExpired):
                    candidate.main()
            self.assertFalse(output.exists())

    def test_report_requires_all_three_runs(self):
        with tempfile.TemporaryDirectory() as folder:
            output = Path(folder) / 'result.json'
            argv = ['candidate', '--sha', SHA, '--ci', '123', '--windows', '123',
                    '--android', '123', '--output', str(output)]
            runs = []
            for workflow in candidate.WORKFLOWS.values():
                runs.append(subprocess.CompletedProcess([], 0, json.dumps(dict(RUN, path='.github/workflows/' + workflow))))
                runs.append(subprocess.CompletedProcess([], 0, json.dumps([{'jobs': [{'name': 'test', 'status': 'completed', 'conclusion': 'success'}]}])))
            with patch.object(sys, 'argv', argv), patch.object(candidate.subprocess, 'run', side_effect=runs):
                candidate.main()
            self.assertEqual(len(json.loads(output.read_text())['runs']), 3)


if __name__ == '__main__':
    unittest.main()
