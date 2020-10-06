#include <string>
#include <random>

#include "nlohmann/json.hpp"
#include "HTTPLib/HTTPHandler.h"
#include "HTTPLib/Server.h"
#include "PG/PGPool.h"
#include "config_handler.h"
#include "PG/DBModel.h"
#include "PG/PGDb.h"

using json = nlohmann::json;

namespace ph {
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

        void to_json(json &j) const {
            j = json{{"id", this->getId()}, {"model", this->getModel()}, {"brand", this->getBrand()}};
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
    auto db = std::make_shared<datasource::PGDb>("127.0.0.1", 5432, config.dbName.c_str(),
                                                 config.dbUser.c_str(), config.dbPass.c_str(), 16);

    // Launch server
    auto *httpHandler = new CPPHTTP::HTTPHandler(16);
    httpHandler->addResource("/", CPPHTTP::GET, [&](CPPHTTP::Request *req, CPPHTTP::Response *resp) {
        resp->headers.emplace("Content-Type", "application/json");

        std::vector<ph::Phone> phones;
        db->select<ph::Phone>("phone", phones);
        json j_arr;
        for (auto &phone: phones) {
            json j;
            phone.to_json(j);
            j_arr.push_back(j);
        }
        resp->body = j_arr.dump();

    });
    httpHandler->addResource("/random", CPPHTTP::GET, [&](CPPHTTP::Request *req, CPPHTTP::Response *resp) {
        resp->headers.emplace("Content-Type", "application/json");

        if (req->method == CPPHTTP::GET) {
            std::random_device r;
            std::default_random_engine e1(r());
            std::uniform_int_distribution<int> uniform_dist(1, 100);
            auto mean = uniform_dist(e1);
            json j = {{"num", mean}};
            resp->body = j.dump();
        } else {
            json j = {{"error", "method not implemented"}};
            resp->body.assign(j.dump());
        }
    });


    CPPHTTP::Server serverSocket(8080, 16386, 16386, 1, httpHandler);

    serverSocket.listenAndServe();
    return 0;
}
