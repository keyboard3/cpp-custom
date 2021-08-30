/* <DESC>
 * Very simple HTTP GET
 * 来源：https://curl.se/libcurl/c/simple.html
 * </DESC>
 */
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct {
  char *buf;
  size_t size;
} memory;

/*使用回调而不用默认为了保证兼容性*/
size_t grow_buffer(void *contents, size_t sz, size_t nmemb, void *ctx) {
  size_t realsize = sz * nmemb;
  memory *mem = (memory *)ctx;
  char *ptr = (char *)realloc(mem->buf, mem->size + realsize);
  if (!ptr) {
    /* out of memory */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
  mem->buf = ptr;
  memcpy(&(mem->buf[mem->size]), contents, realsize);
  mem->size += realsize;
  return realsize;
}
int main(void) {
  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://raw.githubusercontent.com/keyboard3/cpp-custom/main/skia/jsGUI/index.html");
    /* example.com 被重定向，所以我们告诉 libcurl 跟随重定向 */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    memory *mem = (memory *)malloc(sizeof(memory));
    mem->size = 0;
    mem->buf = (char *)malloc(1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, grow_buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, mem);
    curl_easy_setopt(curl, CURLOPT_PRIVATE, mem);

    /* 执行请求， res 会得到返回码 */
    res = curl_easy_perform(curl);
    /* 检查错误 */
    if (res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    printf("%d %s\n", mem->size,mem->buf);
    /* 记得清理 */
    curl_easy_cleanup(curl);
  }
  return 0;
}