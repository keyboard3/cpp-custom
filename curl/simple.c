/* <DESC>
 * Very simple HTTP GET
 * 来源：https://curl.se/libcurl/c/simple.html
 * </DESC>
 */
#include <stdio.h>
#include <curl/curl.h>
 
int main(void)
{
  CURL *curl;
  CURLcode res;
 
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://example.com");
    /* example.com 被重定向，所以我们告诉 libcurl 跟随重定向 */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
 
    /* 执行请求， res 会得到返回码 */
    res = curl_easy_perform(curl);
    /* 检查错误 */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    /* 记得清理 */
    curl_easy_cleanup(curl);
  }
  return 0;
}