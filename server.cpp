#include <string>
#include <random>
#include "Logger/SimpleLogger.h"

#include "nlohmann/json.hpp"
#include "HTTPLib/HTTPHandler.h"
#include "HTTPLib/Server.h"
#include "PG/PGPool.h"
#include "config_handler.h"
#include "PG/DBModel.h"
#include "PG/PGDb.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

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

        void setId(long long int id) {
            this->m_id = id;
        }

        [[nodiscard]] const std::string &getModel() const {
            return m_model;
        }

        void setModel(const std::string &model) {
            this->m_model = model;
        }

        [[nodiscard]] const std::string &getBrand() const {
            return m_brand;
        }

        void setBrand(const std::string &brand) {
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

        [[nodiscard]] Value &to_json() const {
            Document d;
            Value obj(kObjectType);
            auto &allocator = d.GetAllocator();
            Value v;
            v.SetInt(this->getId());
            obj.AddMember("id", v, allocator);
            v.SetString(this->getModel().c_str(), this->getModel().size(), allocator);
            obj.AddMember("model", v, allocator);
            v.SetString(this->getBrand().c_str(), this->getBrand().size(), allocator);
            obj.AddMember("brand", v, allocator);
            return obj.Move();
        }

        void to_json(Document &d) const {
            if (d.IsArray()) {
                Value obj(kObjectType);
                auto &allocator = d.GetAllocator();
                Value v;
                v.SetInt(this->getId());
                obj.AddMember("id", v, allocator);
                v.SetString(this->getModel().c_str(), this->getModel().size(), allocator);
                obj.AddMember("model", v, allocator);
                v.SetString(this->getBrand().c_str(), this->getBrand().size(), allocator);
                obj.AddMember("brand", v, allocator);
                d.PushBack(obj, allocator);
            } else {
                throw std::runtime_error("rapidjson::Document should be array");
            }
        }

        void from_json(json &j) {
            this->setId(j.at("id").get<long long>());
            this->setModel(j.at("model").get<std::string>());
            this->setBrand(j.at("brand").get<std::string>());
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
    CPPHTTP::HTTPHandler httpHandler(16);
    httpHandler.addResource("/", CPPHTTP::GET, [&](CPPHTTP::Request *req, CPPHTTP::Response *resp) {

        resp->headers.emplace("Content-Type", "application/json");
        Document d(rapidjson::kArrayType);

        auto &allocator = d.GetAllocator();
        std::vector<ph::Phone> phones;
        db.Find<ph::Phone>("phone", phones);

        for (auto &phone: phones) {
            phone.to_json(d);
        }
        buffer.Clear();
        Writer<StringBuffer> writer(buffer);
        d.Accept(writer);
        resp->body = std::string(buffer.GetString());

    });
    httpHandler.addResource("/random", CPPHTTP::GET, [&](CPPHTTP::Request *req, CPPHTTP::Response *resp) {
        resp->headers.emplace("Content-Type", "application/json");
        Document d(rapidjson::kObjectType);
        auto &allocator = d.GetAllocator();
        if (req->method == CPPHTTP::GET) {
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


    CPPHTTP::Server serverSocket(8080, 16386, 16386, 1, httpHandler, logger);

    serverSocket.listenAndServe();
    return 0;
}
