#include <iostream>

#include <Poco/Net/X509Certificate.h>
#include <Poco/Util/ServerApplication.h>

using namespace std;
using namespace Poco;
using namespace Poco::Util;

class SimpleFileServer : public Poco::Util::ServerApplication {
   private:
   public:
    SimpleFileServer() { Poco::Net::initializeSSL(); }

    virtual ~SimpleFileServer() { Poco::Net::uninitializeSSL(); }

   protected:
    void initialize(Application& self) {
        loadConfiguration();
        ServerApplication::initialize(self);
    }

    void uninitialize() { ServerApplication::uninitialize(); }

    virtual void defineOptions(OptionSet& options) {
        ServerApplication::defineOptions(options);
    }

    void handleOption(const std::string& name, const std::string& value) {
        ServerApplication::handleOption(name, value);
    }

    virtual int main(const std::vector<std::string>& args) {
        cout << "Hello" << endl;

        return 0;
    }
};

POCO_SERVER_MAIN(SimpleFileServer)

