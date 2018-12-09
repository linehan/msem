#ifndef JLIB_WCHAR_H
#define JLIB_WCHAR_H

wchar_t *wcs_dup(const wchar_t *wcs);
void     wcs_dimensions(const wchar_t *wcs, int *w, int *h);
wchar_t *wcs_wrap(wchar_t *dst, wchar_t *src, int width);

#endif
