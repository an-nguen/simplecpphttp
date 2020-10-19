#include <string>
#include <random>

#include "config_handler.h"

#include "HTTPLib/HTTPHandler.h"
#include "HTTPLib/Server.h"

#include "Logger/SimpleLogger.h"

#include "PG/DBModel.h"
#include "PG/PGDb.h"
#include "PG/PGPool.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace ph {
    using namespace rapidjson;

    class Phone : DBModel {
    public:
        Phone() = default;

        Phone(long long int id, std::string model, std::string brand) : m_id(id), m_model(std::move(model)),
                                                                        m_brand(std::move(brand)) {}

        [[nodiscard]] long long int getId() const {
            return m_id;
        }

        [[maybe_unused]] void setId(long long int id) {
            this->m_id = id;
        }

        [[nodiscard]] const std::string &getModel() const {
            return m_model;
        }

        [[maybe_unused]] void setModel(const std::string &model) {
            this->m_model = model;
        }

        [[nodiscard]] const std::string &getBrand() const {
            return m_brand;
        }

        [[maybe_unused]] void setBrand(const std::string &brand) {
            this->m_brand = brand;
        }

        void init(std::map<std::string, std::string> map) override {
            m_id = std::stol(map.at("id"));
            m_model = map.at("model");
            m_brand = map.at("brand");
        }

        [[nodiscard]] std::string getTableName() const override {
            return std::string("phone");
        }

        void toJson(Document &d) const {
            if (d.IsArray()) {
                Value obj(kObjectType);
                auto &allocator = d.GetAllocator();
                Value v;
                v.SetInt(int(this->getId()));
                obj.AddMember("id", v, allocator);
                v.SetString(this->getModel().c_str(), unsigned (this->getModel().size()), allocator);
                obj.AddMember("model", v, allocator);
                v.SetString(this->getBrand().c_str(), unsigned (this->getBrand().size()), allocator);
                obj.AddMember("brand", v, allocator);
                d.PushBack(obj, allocator);
            } else {
                throw std::runtime_error("rapidjson::Document should be array");
            }
        }

        friend std::ostream &operator<<(std::ostream &os, const Phone &obj) {
            os << "Phone{" << obj.m_id << "," << obj.m_model << "," << obj.m_brand << "}";
            return os;
        }

    private:
        long long m_id{};
        std::string m_model;
        std::string m_brand;
    };


}

int main() {
    using rapidjson::Document;
    using rapidjson::StringBuffer;
    using rapidjson::Writer;
    using rapidjson::Value;
    StringBuffer buffer;
    SimpleLogger logger;

    // Read config file
    configs::DBConfig config{};
    json jsonConfig;
    int res = configs::readConfig("./config.json", &jsonConfig, config);
    if (res < -1) {
        return -1;
    } else {
        std::cout << "Config fetched!" << std::endl;
    }


    // Create pq pool
    datasource::PGDb db("::1", 5432, config.dbName.c_str(),
                        config.dbUser.c_str(), config.dbPass.c_str(), 16, logger);

    // Launch server
    cpphttp::HTTPHandler httpHandler(16, logger);
    httpHandler.addResource("/", cpphttp::GET, [&](std::shared_ptr<cpphttp::Request> &req, std::shared_ptr<cpphttp::Response> &resp) {
        resp->headers.emplace("Content-Type", "application/json");
        Document d(rapidjson::kArrayType);
        std::vector<ph::Phone> phones;
        db.Find<ph::Phone>("phone", phones);

        for (auto &phone: phones) {
            phone.toJson(d);
        }
        buffer.Clear();
        Writer<StringBuffer> writer(buffer);
        d.Accept(writer);
        resp->body = std::string(buffer.GetString());

    });
    httpHandler.addResource("/random", cpphttp::GET, [&](std::shared_ptr<cpphttp::Request> &req, std::shared_ptr<cpphttp::Response> &resp) {
        resp->headers.emplace("Content-Type", "application/json");
        Document d(rapidjson::kObjectType);
        auto &allocator = d.GetAllocator();
        if (req->method == cpphttp::GET) {
            std::random_device r;
            std::default_random_engine e1(r());
            std::uniform_int_distribution<int> uniform_dist(1, 100);
            auto mean = uniform_dist(e1);
            buffer.Clear();
            Writer<StringBuffer> writer(buffer);
            d.AddMember("num", Value().SetInt(mean), allocator);
            d.Accept(writer);
            resp->body = std::string(buffer.GetString());
        } else {
            buffer.Clear();
            Writer<StringBuffer> writer(buffer);
            d.AddMember("error", "method not implemented", allocator);
            d.Accept(writer);
            resp->body.assign(std::string(buffer.GetString()));
        }
    });


    cpphttp::Server serverSocket(8080, 16386, 16386, 1, httpHandler, logger);

    serverSocket.listenAndServe();
    return 0;
}
