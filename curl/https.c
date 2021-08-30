/* <DESC>
 * Simple HTTPS GET
 * 来源：https://curl.se/libcurl/c/https.html
 * </DESC>
 */
#include <curl/curl.h>
#include <stdio.h>

int main(void) {
  CURL *curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://example.com");

#ifdef SKIP_PEER_VERIFICATION
    /*
        如果要连接到的站点不使用由您拥有的 CA
    包中的某个证书签名的证书，则可以跳过服务器证书的验证。这使得连接变得不那么安全。
        如果您将服务器的 CA 证书存储在默认包之外的其他位置，则 CURLOPT_CAPATH
    选项可能会派上用场
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
    如果您要连接的站点使用的主机名与他们在服务器证书的 commonName（或
    subjectAltName）字段中提到的不同，libcurl
    将拒绝连接。您可以跳过此检查，但这会降低连接的安全性。
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

    /* 执行请求， res 会得到返回码 */
    res = curl_easy_perform(curl);
    /* 检查错误 */
    if (res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* 清理 */
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  return 0;
}