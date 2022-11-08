#include "SimpleSocket.hpp"
#include "BindingSocket.hpp"
#include "ListeningSocket.hpp"
#include "ConnectingSocket.hpp"

using namespace std;

int main() {

    cout << "Starting... " << endl;
    cout << "Binding Socket..." << endl;
    HDE::BindingSocket b = HDE::BindingSocket(AF_INET, SOCK_STREAM, 0, 8081, INADDR_ANY);
    cout << "Listening Socket..." << endl;
    HDE::ListeningSocket ls = HDE::ListeningSocket(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY, 10);
    cout << "Success..." << endl;
}