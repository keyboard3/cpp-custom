/* <DESC>
 * Basic URL API use.
 * 来源：https://curl.se/libcurl/c/parseurl.html
 * </DESC>
 */
#include <stdio.h>
#include <curl/curl.h>
 
#if !CURL_AT_LEAST_VERSION(7, 62, 0)
#error "this example requires curl 7.62.0 or later"
#endif
 
int main(void)
{
  CURLU *h;
  CURLUcode uc;
  char *host;
  char *path;
 
  h = curl_url(); /* get a handle to work with */
  if(!h)
    return 1;
 
  /* parse a full URL */
  uc = curl_url_set(h, CURLUPART_URL, "http://example.com/path/index.html", 0);
  if(uc)
    goto fail;
 
  /* 从解析的 URL 中提取主机名 */
  uc = curl_url_get(h, CURLUPART_HOST, &host, 0);
  if(!uc) {
    printf("Host name: %s\n", host);
    curl_free(host);
  }
 
  /* 从解析的 URL 中提取路径 */
  uc = curl_url_get(h, CURLUPART_PATH, &path, 0);
  if(!uc) {
    printf("Path: %s\n", path);
    curl_free(path);
  }
 
  /* 使用相对 URL 重定向 */
  uc = curl_url_set(h, CURLUPART_URL, "../another/second.html", 0);
  if(uc)
    goto fail;
 
  /* 提取新的、更新的路径 */
  uc = curl_url_get(h, CURLUPART_PATH, &path, 0);
  if(!uc) {
    printf("Path: %s\n", path);
    curl_free(path);
  }
 
  fail:
  curl_url_cleanup(h); /* free url handle */
  return 0;
}