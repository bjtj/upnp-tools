#include <iostream>
#include <liboslayer/os.hpp>
#include <liboslayer/Text.hpp>
#include <libupnp-tools/Uuid.hpp>

using namespace std;
using namespace OS;
using namespace UTIL;
using namespace UPNP;

void s_valid() {
    //string sample = "12345678-1234-abcd-ef12-1122aabbccdd";
    string sample = "2fac1234-31f8-11b4-a222-08002b34c003";
    Uuid::validFormat(sample);
    cout << "valid" << endl;
}

char s_rand_hex() {
    char r = rand() % 16;
    char ret[2] = {0,};
    snprintf(ret, sizeof(ret), "%x", r);
    return ret[0];
}

string s_hex(size_t cnt) {
    
    string ret;
    
    for (size_t i = 0; i < cnt; i++) {
        char c = s_rand_hex();
        ret.append(1, c);
    }
    
    return ret;
}

string s_gen_uuid() {
    return s_hex(8) + "-" + s_hex(4) + "-" + s_hex(4) + "-" + s_hex(4) + "-" + s_hex(12);
}

void s_test_gen_uuid() {
    for (size_t i = 0; i < 100; i++) {
        cout << s_gen_uuid() << endl;
    }
}

void s_tick_hex() {
    string ret = Text::toHexString(tick_milli());
    if (ret.length() < 12) {
        ret = string(12 - ret.length(), '0') + ret;
    }
    cout << ret << endl;
}

int main(int argc, char * args[]) {
    
//    s_valid();

//    s_test_gen_uuid();
    
//    for (size_t i = 0; i < 1000; i++) {
//        s_tick_hex();
//        idle(10);
//    }
    
    for (size_t i = 0; i < 0xfffff; i++) {
        cout << Uuid::generateUuid() << endl;
    }

	getchar();
    
    return 0;
}