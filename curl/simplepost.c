/* <DESC>
 * Very simple HTTP POST
 * 来源：https://curl.se/libcurl/c/simplepost.html
 * </DESC>
 */
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
 
int main(void)
{
  CURL *curl;
  CURLcode res;
 
  static const char *postthis = "moo mooo moo moo";
 
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://example.com");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postthis);
 
    /* 如果我们不提供 POSTFIELDSIZE，libcurl 将用自己用 strlen() */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(postthis));
 
    /* 执行请求， res 会得到返回码 */
    res = curl_easy_perform(curl);
    /* 检查错误 */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  return 0;
}