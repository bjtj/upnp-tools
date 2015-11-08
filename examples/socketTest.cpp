#include <liboslayer/os.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace OS;

void test() {
    int ret;
    ServerSocket server(8084);
    server.setReuseAddr();
    ret = server.bind();
    cout << "bind() / ret: " << ret << endl;
    
    ret = server.listen(5);
    cout << "listen() / ret: " << ret << endl;
}

void test2() {
    
    DatagramSocket sock(1900);
    sock.setReuseAddr();
    sock.joinGroup("239.255.255.250");
    
    char buffer[4096] = {0,};
    DatagramPacket packet(buffer, sizeof(buffer));
    
    while (1) {
        sock.recv(packet);
        string str(packet.getData(), packet.getLength());
        cout << str << endl;
        
        sockaddr_in sock_addr;
        socklen_t sock_len;
        if (::getsockname(sock.getFd(), (struct sockaddr*)&sock_addr, &sock_len) < 0) {
            perror("getsockname() error");
        } else {
            string ip = InetAddress::getIPAddress(&sock_addr);
            cout << "IP: " << ip << endl;
        }
        
    }
}

int main(int argc, char * args[]) {
    
    test2();
    
    return 0;
}