#include "Crypto.h"

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <iomanip>

#include "Utils.h"

#include <cstring>


void Crypto::pkcs7_pad(unsigned char* data, size_t data_len, size_t block_size) {
     size_t padding_len = block_size - data_len;
     for(size_t i = data_len; i < block_size; i++)
             data[i] = static_cast<unsigned char> (padding_len);
}

std::string Crypto::md5_encrypt(const std::string& key, const std::string& p, const std::string& IV) {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    std::string concat_str = key + IV;
    unsigned char md5_value [EVP_MAX_MD_SIZE];
    unsigned int md5_len, i;

    md = EVP_md5();
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex2(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, concat_str.c_str(), size(concat_str));
    EVP_DigestFinal_ex(mdctx, md5_value, &md5_len);

    EVP_MD_CTX_free(mdctx);

    /*
    printf("Digest is: ");
    for(int i=0; i<md5_len; i++)
           printf("%02x", md5_value[i]);
    printf("\n");
    */

    unsigned char p_padded[16];
    std::memcpy(p_padded, p.c_str(), 16);
    pkcs7_pad(p_padded, p.size(), 16);

    unsigned char xor_result [16];
    for(int i=0; i<16; i++)
           xor_result[i] = md5_value[i] ^ p_padded[i];
    /*
    printf("XOR result is: ");
    for(int i=0; i<16; i++)
           printf("%02x", xor_result[i]);

    printf("\n");
    */

    return std::string(reinterpret_cast<char*>(xor_result), 16);
}

std::string Crypto::md5_decrypt(const std::string& key,const std::string& p, const std::string& IV) {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    std::string concat_str = key + IV;
    unsigned char md5_value [EVP_MAX_MD_SIZE];
    unsigned int md5_len, i;

    md = EVP_md5();
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex2(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, concat_str.c_str(), size(concat_str));
    EVP_DigestFinal_ex(mdctx, md5_value, &md5_len);

    EVP_MD_CTX_free(mdctx);

    /*
    printf("Digest is: ");
    for(int i=0; i<md5_len; i++)
           printf("%02x", md5_value[i]);
    printf("\n");
    */

    unsigned char decrypted[16];
    for(int i=0; i<16; i++)
           decrypted[i] = md5_value[i] ^ p[i];

    return std::string(reinterpret_cast<char*>(decrypted), 16 - decrypted[15]);
}

std::string Crypto::custom_encrypt(const std::string& key, const std::string& p, const std::string& IV) {
    std::string result;
    std::string c_i = IV;

    size_t block_size = 15;
    for(size_t i=0; i<p.size(); i+=block_size) {
         size_t length = std::min(block_size, p.size() - i);
         c_i = md5_encrypt(key, p.substr(i, length), c_i);
         result += c_i;
    }

    return result;
}

std::string Crypto::custom_decrypt(const std::string& key, const std::string& c, const std::string & IV) {
    std::string result;
    std::string c_i = IV;

    for(size_t i=0; i<c.size(); i+=16) {
        std::string chunk = c.substr(i, 16);
        result += md5_decrypt(key, chunk, c_i);
        c_i = chunk;
    }

    return result;
}

RSA* Crypto::load_private_key_from_file(const std::string& filename) 
{
    FILE* privkey_file = fopen(filename.c_str(), "r");
    RSA* rsa = PEM_read_RSAPrivateKey(privkey_file, nullptr, nullptr, nullptr);
    fclose(privkey_file);
    return rsa;
}

RSA* Crypto::load_public_key_from_file(const std::string& filename) 
{
    FILE* pubkey_file = fopen(filename.c_str(), "r");
    RSA* rsa = PEM_read_RSA_PUBKEY(pubkey_file, nullptr, nullptr, nullptr);
    fclose(pubkey_file);
    return rsa;
}


void Crypto::printEncryption(std::vector<unsigned char> digest)
{
    printf("Encrypted digest is: ");
    for(int i=0; i<digest.size(); i++)
           printf("%02x", digest[i]);
    printf("\n");
}
