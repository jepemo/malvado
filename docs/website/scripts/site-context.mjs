export const version = process.env.MALVADO_DOCS_VERSION;
export const revision = process.env.MALVADO_DOCS_REVISION;
export const repository = 'https://github.com/jepemo/malvado';

export function repoLink(path, kind = 'blob') {
  return `${repository}/${kind}/${revision || 'master'}/${path}`;
}
