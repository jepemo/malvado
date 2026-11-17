#!/usr/bin/env python3
"""Require successful supported-platform workflow runs for one exact revision."""
# Malvado 2; SPDX-License-Identifier: Apache-2.0
import argparse
import json
import re
import subprocess
from pathlib import Path

WORKFLOWS = {'ci': 'ci.yml', 'windows': 'windows.yml', 'android': 'android.yml'}


def validate_run(run, sha, workflow, repository):
    if not re.fullmatch(r'[0-9a-f]{40}', sha):
        raise ValueError('Candidate must be a full lowercase commit SHA')
    if run.get('head_sha') != sha:
        raise ValueError('Workflow revision does not match the candidate')
    if run.get('path') != '.github/workflows/' + workflow:
        raise ValueError('Unexpected workflow path')
    if run.get('repository', {}).get('full_name') != repository:
        raise ValueError('Unexpected workflow repository')
    if run.get('head_repository', {}).get('full_name') != repository:
        raise ValueError('Fork runs cannot establish candidate acceptance')
    if run.get('event') not in ('push', 'workflow_dispatch', 'schedule'):
        raise ValueError('Candidate requires a branch workflow run')
    if run.get('status') != 'completed' or run.get('conclusion') != 'success':
        raise ValueError('Workflow must be completed successfully')
    return {'id': run['id'], 'url': run['html_url'], 'workflow': workflow,
            'sha': sha, 'attempt': run['run_attempt'],
            'event': run['event'], 'conclusion': 'success'}


def validate_jobs(pages):
    jobs = [job for page in pages for job in page['jobs']]
    if not jobs or any(job.get('status') != 'completed' or
                       job.get('conclusion') != 'success' for job in jobs):
        raise ValueError('Every workflow job must complete successfully')
    return [{'name': job['name'], 'conclusion': 'success'} for job in jobs]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sha', required=True)
    parser.add_argument('--repository', default='jepemo/malvado')
    parser.add_argument('--output', type=Path, required=True)
    for name in WORKFLOWS:
        parser.add_argument('--' + name, required=True, type=int)
    args = parser.parse_args()
    # Never retain a previous success report after a failed verification.
    args.output.unlink(missing_ok=True)
    evidence = []
    for name, workflow in WORKFLOWS.items():
        run_id = getattr(args, name)
        if run_id <= 0:
            raise ValueError('Run IDs must be positive')
        result = subprocess.run(['gh', 'api', f'repos/{args.repository}/actions/runs/{run_id}'],
                                check=True, capture_output=True, text=True, timeout=60)
        run = json.loads(result.stdout)
        if run.get('id') != run_id:
            raise ValueError('Workflow run ID mismatch')
        record = validate_run(run, args.sha, workflow, args.repository)
        jobs = subprocess.run(['gh', 'api', '--paginate', '--slurp',
                               f'repos/{args.repository}/actions/runs/{run_id}/attempts/{run["run_attempt"]}/jobs?per_page=100'],
                              check=True, capture_output=True, text=True, timeout=60)
        record['jobs'] = validate_jobs(json.loads(jobs.stdout))
        evidence.append(record)
    args.output.write_text(json.dumps({'sha': args.sha, 'repository': args.repository,
                                      'runs': evidence}, indent=2) + '\n')
    print(f'All supported-platform workflows passed on {args.sha}')


if __name__ == '__main__':
    main()
