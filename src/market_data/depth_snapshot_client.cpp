#include "market_data/depth_snapshot_client.hpp"
#include "market_data/depth_parser.hpp"

#include <curl/curl.h>

#include <string>

namespace trading {
namespace {

size_t write_callback(
    char* data,
    size_t size,
    size_t count,
    void* user_data)
{
    const auto bytes = size * count;
    auto* response = static_cast<std::string*>(user_data);

    response->append(data, bytes);
    return bytes;
}

} // namespace

std::optional<DepthSnapshot> DepthSnapshotClient::fetch(
    const std::string& symbol)
{
    const std::string url =
        "https://testnet.binance.vision/api/v3/depth"
        "?symbol=" + symbol + "&limit=1000";

    CURL* curl = curl_easy_init();

    if (!curl) {
        return std::nullopt;
    }

    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    const CURLcode result = curl_easy_perform(curl);

    long status_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);

    curl_easy_cleanup(curl);

    if (result != CURLE_OK || status_code != 200) {
        return std::nullopt;
    }

    auto snapshot = DepthParser::parse_snapshot(response);

    if (!snapshot) {
        return std::nullopt;
    }

    // Binance REST depth snapshots do not include the symbol.
    snapshot->symbol = symbol;

    return snapshot;
}

} // namespace trading
