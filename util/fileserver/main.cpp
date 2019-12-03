#include <iostream>

#include <Poco/File.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/ConsoleCertificateHandler.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerRequestImpl.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/KeyConsoleHandler.h>
#include <Poco/Net/KeyFileHandler.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/SecureServerSocket.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Net/X509Certificate.h>
#include <Poco/URI.h>
#include <Poco/Util/Application.h>
#include <Poco/Util/ServerApplication.h>

using namespace std;
using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;

class Http404Handler : public HTTPRequestHandler {
   public:
    Http404Handler() {}
    void handleRequest(HTTPServerRequest& request,
                       HTTPServerResponse& response) {
        response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
        response.send();
    }
};

class FileRequestHandler : public HTTPRequestHandler {
   public:
    FileRequestHandler() {
        // Application& app = Application::instance();
        // app.logger().information("Ctor: this is %LX", (uint64_t)this);
    }

    void handleRequest(HTTPServerRequest& request,
                       HTTPServerResponse& response) {
        Application& app = Application::instance();
        app.logger().information("Request from " +
                                 request.clientAddress().toString());

        string path = request.getURI().substr(1);
        URI uri(request.getURI());
        Path requestPath("./", uri.getPath());

        response.setContentType("text/html");

        ostream& ostr = response.send();
        ostr << "<html>\n";
        ostr << "<head><title>"
             << "File request"
             << "</title></head>\n";
        ostr << "<body>";
        ostr << "You Requested<br>";
        ostr << "Path: " << requestPath.toString() << "<br>" << endl;
        ostr << "filename: " << requestPath.getFileName() << "<br>" << endl;
        ostr << "isFile: " << boolalpha << requestPath.isFile() << "<br>"
             << endl;
        ostr << "isDirectory: " << boolalpha << requestPath.isDirectory()
             << "<br>" << endl;
        File f(requestPath);
        try {
            bool exist = f.exists();
            ostr << "exist: " << boolalpha << exist << "<br>" << endl;
            size_t size = f.getSize();
            ostr << "Size: " << size << "<br>" << endl;
        } catch (std::exception& ex) {
            string s = ex.what();
            app.logger().information("%s", s);
        }

        ostr << "</body>";
        ostr << "</html>";
    }
};

class FileRequestHandlerFactory : public HTTPRequestHandlerFactory {
   public:
    FileRequestHandlerFactory() {}

    HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request) {
        HTTPRequestHandler* ret = nullptr;

        string method = request.getMethod();
        URI uri(request.getURI());
        string host = request.getHost();

        Application& app = Application::instance();
        app.logger().information("Request: %s, %s, %s", host, method, uri.toString());

        if (uri.getPath().find("/") == 0) {
            ret = new FileRequestHandler();
        } else {
            ret = new Http404Handler();
        }

        return ret;
    }
};

class SimpleFileServer : public Poco::Util::ServerApplication {
   private:
   public:
    SimpleFileServer() {
        SharedPtr<PrivateKeyPassphraseHandler> pConsoleHandler =
            new KeyConsoleHandler(true);
        SharedPtr<InvalidCertificateHandler> pInvalidCertHandler =
            new ConsoleCertificateHandler(true);
        Context::Ptr pContext =
            new Context(Context::SERVER_USE,
                        "/etc/letsencrypt/live/ical197.ddns.net/privkey.pem",
                        "/etc/letsencrypt/live/ical197.ddns.net/cert.pem",
                        "/etc/letsencrypt/live/ical197.ddns.net/fullchain.pem",
                        Context::VERIFY_NONE, 9, false,
                        "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
        SSLManager::instance().initializeServer(pConsoleHandler,
                                                pInvalidCertHandler, pContext);

        Poco::Net::initializeSSL();
    }

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
        int port = 9443;

        SecureServerSocket svs(port);
        HTTPServer svr(new FileRequestHandlerFactory(), svs,
                       new HTTPServerParams());
        svr.start();
        waitForTerminationRequest();
        svr.stop();

        return 0;
    }
};

POCO_SERVER_MAIN(SimpleFileServer)

