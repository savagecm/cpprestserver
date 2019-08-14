//
//  Created by Ivan Mejia on 12/24/16.
//
// MIT License
//
// Copyright (c) 2016 ivmeroLabs.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#pragma once
#include <cpprest/http_msg.h>
#include <basic_controller.hpp>
#include "logger/logger.hpp"

using namespace cfx;

class MicroserviceController : public BasicController, Controller
{
public:
    MicroserviceController() : BasicController() {}
    ~MicroserviceController() {}
    void handleGet(http_request message) override
    {

        if (CHECK_LOG_LEVEL(debug))
        {
            __LOG(debug, "get a get message : " << message.to_string());
        }

        auto path = requestPath(message);
        if (!path.empty())
        {
            if (path[0] == "service" && path[1] == "test")
            {
                auto response = json::value::object();
                response["version"] = json::value::string("0.1.1");
                response["status"] = json::value::string("ready!");
                message.reply(status_codes::OK, response);
            }
        }

        message.reply(status_codes::NotFound);
    }
    void handlePut(http_request message) override
    {
        message.reply(status_codes::NotImplemented, responseNotImpl(methods::PUT));
    }
    void handlePost(http_request message) override
    {
        if (CHECK_LOG_LEVEL(debug))
        {
            __LOG(debug, "get a post message : " << message.to_string());
        }
        auto path = requestPath(message);
        if (!path.empty())
        {
            if (path[0] == "image")
            {
                web::json::value jValue;
                try
                {
                    jValue = message.extract_json().get();
                }
                catch (const http_exception &e)
                {
                    // Print error.
                    if (CHECK_LOG_LEVEL(debug))
                    {
                        __LOG(debug, "invalid json format, error is : " << e.what());
                    }
                    message.reply(status_codes::BadRequest);
                    return;
                }
                switch (ProcessImage(jValue, path, methods::POST))
                {
                case imageRet::SUCCESS:
                    message.reply(status_codes::OK, jValue);
                    break;
                case imageRet::NOT_SUPPORT:
                    message.reply(status_codes::NotFound);
                    break;
                case imageRet::BAD_REQUEST:
                    message.reply(status_codes::BadRequest);
                    break;
                default:
                    message.reply(status_codes::BadRequest);
                }
            }
            else
            {
                message.reply(status_codes::NotFound);
            }
        }
        else
        {
            message.reply(status_codes::NotFound);
        }
    }

    void handlePatch(http_request message) override
    {
        message.reply(status_codes::NotImplemented, responseNotImpl(methods::PATCH));
    }
    void handleDelete(http_request message) override
    {
        message.reply(status_codes::NotImplemented, responseNotImpl(methods::DEL));
    }
    void handleHead(http_request message) override
    {
        message.reply(status_codes::NotImplemented, responseNotImpl(methods::HEAD));
    }
    void handleOptions(http_request message) override
    {
        message.reply(status_codes::NotImplemented, responseNotImpl(methods::OPTIONS));
    }
    void handleTrace(http_request message) override
    {
        message.reply(status_codes::NotImplemented, responseNotImpl(methods::TRCE));
    }
    void handleConnect(http_request message) override
    {
        message.reply(status_codes::NotImplemented, responseNotImpl(methods::CONNECT));
    }
    void handleMerge(http_request message) override
    {
        message.reply(status_codes::NotImplemented, responseNotImpl(methods::MERGE));
    }
    void initRestOpHandlers() override
    {
        _listener.support(methods::GET, std::bind(&MicroserviceController::handleGet, this, std::placeholders::_1));
        _listener.support(methods::PUT, std::bind(&MicroserviceController::handlePut, this, std::placeholders::_1));
        _listener.support(methods::POST, std::bind(&MicroserviceController::handlePost, this, std::placeholders::_1));
        _listener.support(methods::DEL, std::bind(&MicroserviceController::handleDelete, this, std::placeholders::_1));
        _listener.support(methods::PATCH, std::bind(&MicroserviceController::handlePatch, this, std::placeholders::_1));
    }
    enum class imageRet
    {
        SUCCESS = 0,
        NOT_SUPPORT,
        BAD_REQUEST

    };
    imageRet ProcessImage(web::json::value jValue, std::vector<utility::string_t> path, const http::method &method)
    {
        if (CHECK_LOG_LEVEL(debug))
        {
            __LOG(debug, "in function ProcessImage");
        }
        // if we go into this function. the path is not empty and the path start with image
        if (path.size() > 1)
        {
            if (path[1] == "line")
            {
                if (CHECK_LOG_LEVEL(debug))
                {
                    __LOG(debug, "in the line case");
                }
                //{"colour":0,"positionx":[0,0],"positiony":[100,100]}
                int colour = jValue.at("colour").as_integer();
                json::array posx = jValue.at("positionx").as_array();
                json::array posy = jValue.at("positiony").as_array();
                if (posx.size() != 2 || posy.size() != 2)
                {
                    return imageRet::BAD_REQUEST;
                }
                int posxx = posx.at(0).as_integer();
                int posxy = posx.at(1).as_integer();
                int posyx = posy.at(0).as_integer();
                int posyy = posy.at(1).as_integer();

                if (CHECK_LOG_LEVEL(debug))
                {
                    __LOG(debug, "the line info is : " << posxx << ":" << posxy << ":" << posyx << ":" << posyy << ", colour is : " << colour);
                }
            }
            else if (path[1] == "circle")
            {
                if (CHECK_LOG_LEVEL(debug))
                {
                    __LOG(debug, "in the circle case");
                }
                //{"colour":0,"position":[0,0],"radius":100}

                int colour = jValue.at("colour").as_integer();
                int radius = jValue.at("radius").as_integer();
                json::array pos = jValue.at("position").as_array();

                if (pos.size() != 2)
                {
                    return imageRet::BAD_REQUEST;
                }
                int posx = pos.at(0).as_integer();
                int posy = pos.at(1).as_integer();

                if (CHECK_LOG_LEVEL(debug))
                {
                    __LOG(debug, "the circle info is : " << posx << ":" << posy << ", colour is : " << colour << ", radius is : " << radius);
                }
            }
            else if (path[1] == "rectangle")
            {
                //{"colour":0,"position":[0,0],"height":100,"wide":100}
                // note : position is the top left point of rectangle
                if (CHECK_LOG_LEVEL(debug))
                {
                    __LOG(debug, "in the rectangle case");
                }

                int colour = jValue.at("colour").as_integer();
                int height = jValue.at("height").as_integer();
                int wide = jValue.at("wide").as_integer();
                json::array pos = jValue.at("position").as_array();

                if (pos.size() != 2)
                {
                    return imageRet::BAD_REQUEST;
                }
                int posx = pos.at(0).as_integer();
                int posy = pos.at(1).as_integer();

                if (CHECK_LOG_LEVEL(debug))
                {
                    __LOG(debug, "the rectangle info is : " << posx << ":" << posy << ", colour is : " << colour << ", height is : " << height << ", wide is : " << wide);
                }
            }
            else if (path[1] == "point")
            {
                // {"colour":0,"position":[0,0]}
                if (CHECK_LOG_LEVEL(debug))
                {
                    __LOG(debug, "in the point case");
                }

                int colour = jValue.at("colour").as_integer();

                json::array pos = jValue.at("position").as_array();

                if (pos.size() != 2)
                {
                    return imageRet::BAD_REQUEST;
                }
                int posx = pos.at(0).as_integer();
                int posy = pos.at(1).as_integer();

                if (CHECK_LOG_LEVEL(debug))
                {
                    __LOG(debug, "the point info is : " << posx << ":" << posy << ", colour is : " << colour);
                }
            }
            else
            {
                if (CHECK_LOG_LEVEL(debug))
                {
                    __LOG(debug, "not spuuort path : " << path[1]);
                }
                return imageRet::NOT_SUPPORT;
            }
            // now process post/get.......
            if (method == methods::POST)
            {
            }
            return imageRet::SUCCESS;
        }
        else
        {
            // path.size  ==1 case

            if (CHECK_LOG_LEVEL(debug))
            {
                __LOG(debug, "path size is : " << path.size());
            }
            return imageRet::NOT_SUPPORT;
        }
    }

private:
    static json::value responseNotImpl(const http::method &method)
    {
        auto response = json::value::object();
        response["serviceName"] = json::value::string("C++ Mircroservice Sample");
        response["http_method"] = json::value::string(method);
        return response;
    }
};