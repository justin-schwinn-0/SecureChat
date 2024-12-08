#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rsa.h>

#include <sstream>
#include <iomanip>
#include <iostream>

#include <cstring>

void pkcs7_pad(unsigned char* data, size_t data_len, size_t block_size) {
     size_t padding_len = block_size - data_len;
     for(size_t i = data_len; i < block_size; i++)
             data[i] = static_cast<unsigned char> (padding_len);
}

std::string md5_encrypt(const std::string& key, const std::string& p, const std::string& IV) {
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

std::string md5_decrypt(const std::string& key, std::string& p, const std::string& IV) {
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

std::string custom_encrypt(const std::string& key, const std::string& p, const std::string& IV) {
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

std::string custom_decrypt(const std::string& key, const std::string& c, const std::string & IV) {
    std::string result;
    std::string c_i = IV;

    for(size_t i=0; i<c.size(); i+=16) {
        std::string chunk = c.substr(i, 16);
        result += md5_decrypt(key, chunk, c_i);
        c_i = chunk;
    }

    return result;
}


RSA* load_private_key_from_file(const std::string& filename) {
     FILE* privkey_file = fopen(filename.c_str(), "r");
     RSA* rsa = PEM_read_RSAPrivateKey(privkey_file, nullptr, nullptr, nullptr);
     fclose(privkey_file);
     return rsa;
}

RSA* load_public_key_from_file(const std::string& filename) {
      FILE* pubkey_file = fopen(filename.c_str(), "r");
      RSA* rsa = PEM_read_RSA_PUBKEY(pubkey_file, nullptr, nullptr, nullptr);
      fclose(pubkey_file);
      return rsa;
}


void rsa() {
     std::string msg = "Quick brown fox";
     RSA* private_key = load_private_key_from_file("/home/david/Networks/SecureChat/private.pem");
     RSA* public_key = load_public_key_from_file("/home/david/Networks/SecureChat/public.pem");

     std::unique_ptr<unsigned char[]> encrypted(new unsigned char[RSA_size(public_key)]);
     int encrypted_length = RSA_public_encrypt(
                     msg.size(),
                     reinterpret_cast <const unsigned char*>(msg.c_str()),
                     encrypted.get(),
                     public_key,
                     RSA_PKCS1_PADDING);

    const unsigned char* encrypted_msg = encrypted.get();
    printf("Encrypted message is: ");
    for(int i=0; i<encrypted_length; i++)
           printf("%02x", encrypted_msg[i]);
    printf("\n");


    std::unique_ptr<unsigned char[]> decrypted(new unsigned char[RSA_size(private_key)]);
    int decrypted_length = RSA_private_decrypt(
                   encrypted_length,
                   encrypted.get(),
                   decrypted.get(),
                   private_key,
                   RSA_PKCS1_PADDING);

    std::cout << "Decrypted message: " << std::string(reinterpret_cast<char*>(decrypted.get()),
                    static_cast<std::size_t>(decrypted_length)) << std::endl;

   RSA_free(private_key);
   RSA_free(public_key);
}

int main()
{
    //rsa();

    std::string encrypted = custom_encrypt("Bob",
                    "First line of Harry Potter: “Mr. and Mrs. Dursley of number four, Privet Drive, were proud to say that they were perfectly normal, thank you very much.”", "1234567890abcdef");

    std::cout << "Encrypted:\n" << encrypted << std::endl;
    std::cout << "Decrypted:\n" << custom_decrypt("Bob", encrypted, "1234567890abcdef") << std::endl;
    return 0;
}
