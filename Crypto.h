#ifndef CRYPTO_H
#define CRYPTO_H

#include <openssl/rsa.h>
#include <string>
#include <memory>
#include <vector>
#include "ttmath/ttmath.h"


const std::string DEFAULT_KEY ="bob";
const std::string DEFAULT_KEY_INTEGRITY ="alice";
const std::string DEFAULT_IV ="1234567890abcdef";

class Crypto
{
public:

    static void pkcs7_pad(  unsigned char* data, 
                            size_t data_len, 
                            size_t block_size);

    static std::string md5_encrypt( const std::string& key, 
                                    const std::string& p, 
                                    const std::string& IV);

    static std::string md5_decrypt( const std::string& key, 
                                    const std::string& p, 
                                    const std::string& IV);

    static std::string custom_decrypt(  const std::string& key, 
                                        const std::string& c, 
                                        const std::string & IV);

    static std::string custom_encrypt(  const std::string& key, 
                                        const std::string& p, 
                                        const std::string& IV);

    static RSA* load_private_key_from_file(const std::string& filename); 

    static RSA* load_public_key_from_file(const std::string& filename); 

    static void printEncryption(std::vector<unsigned char> digest);


    template<long unsigned int T>
    static void charVecToNum(const std::vector<unsigned char>& vec,ttmath::UInt<T>& num)
    {
        num.SetZero();
        for(int i = 0; i < vec.size();i++)
        {
            num.AddInt(vec[i] << (i * 8));
        }
    }
};


#endif
