// This file may be redistributed and modified only under the terms of
// the GNU Lesser General Public License (See COPYING for details).
// Copyright (C) 2001 Alistair Riddoch

#include <Atlas/Codec.h>
#include <Atlas/Message/Object.h>
#include <Atlas/Net/Stream.h>
#include <Atlas/Objects/Decoder.h>
#include <Atlas/Objects/Encoder.h>

#include <varconf/Config.h>

#include <skstream.h>

#include "ClientConnection.h"

#include "debug.h"
#include "process.h"

static const bool debug_flag = false;

using Atlas::Message::Object;

static inline const std::string typeAsString(const Object & o)
{
    switch (o.GetType()) {
        case Object::TYPE_NONE:
            return "none";
        case Object::TYPE_INT:
            return "integer";
        case Object::TYPE_FLOAT:
            return "float";
        case Object::TYPE_STRING:
            return "string";
        case Object::TYPE_MAP:
            return "map";
        case Object::TYPE_LIST:
            return "list";
        default:
            return "unknown";
    }
}

ClientConnection::ClientConnection() :
    client_fd(-1), encoder(NULL), serialNo(512)
{
}

ClientConnection::~ClientConnection()
{
    if (encoder != NULL) {
        delete encoder;
    }
}

void ClientConnection::ObjectArrived(const Error&op)
{
    debug(std::cout << "Error" << std::endl << std::flush;);
    push(op);
    reply_flag = true;
    error_flag = true;
}

void ClientConnection::ObjectArrived(const Info & op)
{
    debug(std::cout << "Info" << std::endl << std::flush;);
    push(op);
    const std::string & from = op.GetFrom();
    if (from.empty()) {
        try {
            Object ac = op.GetArgs().front();
            reply = ac.AsMap();
            // const std::string & acid = reply["id"].AsString();
            // objects[acid] = new ClientAccount(acid, *this);
        }
        catch (...) {
        }
    }
}

void ClientConnection::ObjectArrived(const Action& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Appearance& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Combine& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Atlas::Objects::Operation::Communicate& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Create& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Delete& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Disappearance& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Divide& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Atlas::Objects::Operation::Feel& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Get& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Imaginary& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Atlas::Objects::Operation::Listen& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Login& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Logout& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Look& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Move& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Atlas::Objects::Operation::Perceive& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Atlas::Objects::Operation::Perception& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const RootOperation& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Set& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Sight& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Atlas::Objects::Operation::Smell& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Atlas::Objects::Operation::Sniff& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Sound& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Talk& op)
{
    push(op);
}

void ClientConnection::ObjectArrived(const Touch& op)
{
    push(op);
}

int ClientConnection::read() {
    if (ios.is_open()) {
        codec->Poll();
        return 0;
    } else {
        return -1;
    }
}

bool ClientConnection::connect(const std::string & server)
{
    struct sockaddr_in serv_sa;

    memset(&serv_sa, 0, sizeof(serv_sa));
    serv_sa.sin_family = AF_INET;
    serv_sa.sin_port = htons(6767);

    struct hostent * serv_addr = gethostbyname(server.c_str());
    if (serv_addr == NULL) {
        return false;
    }
    memcpy(&serv_sa.sin_addr, serv_addr->h_addr_list[0], serv_addr->h_length);

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        return false;
    }

    int ret;
    ret = ::connect(client_fd, (struct sockaddr *)&serv_sa, sizeof(serv_sa));
    if (ret < 0) {
        return false;
    }

    ios.attach(client_fd);

    Atlas::Net::StreamConnect conn("cyphesis_aiclient", ios, this);

    while (conn.GetState() == Atlas::Net::StreamConnect::IN_PROGRESS) {
      conn.Poll();
    }
  
    if (conn.GetState() == Atlas::Net::StreamConnect::FAILED) {
        return false;
    }

    codec = conn.GetCodec();

    encoder = new Atlas::Objects::Encoder(codec);

    codec->StreamBegin();

    return true;
}

void ClientConnection::close()
{
    ios.close();
    client_fd = -1;
}

bool ClientConnection::login(const std::string & account,
                             const std::string & password)
{
    Atlas::Objects::Operation::Login l = Atlas::Objects::Operation::Login::Instantiate();
    Object::MapType acmap;
    acmap["id"] = account;
    acmap["password"] = password;
    acmap["parents"] = Object::ListType(1,"account");
    acmap["objtype"] = "object";

    acName = account;

    l.SetArgs(Object::ListType(1,Object(acmap)));

    reply_flag = false;
    error_flag = false;
    send(l);
    return true;
}

bool ClientConnection::create(const std::string & account,
                              const std::string & password)
{
    Atlas::Objects::Operation::Create c = Atlas::Objects::Operation::Create::Instantiate();
    Object::MapType acmap;
    acmap["id"] = account;
    acmap["password"] = password;
    acmap["parents"] = Object::ListType(1,"account");
    acmap["objtype"] = "object";

    acName = account;

    c.SetArgs(Object::ListType(1,Object(acmap)));

    reply_flag = false;
    error_flag = false;
    send(c);
    return true;
}

bool ClientConnection::wait(int time)
// Waits for response from server. Used when we are expecting a login response
// Return whether or not an error occured
{
    error_flag = false;
    reply_flag = false;
    debug( std::cout << "WAITing" << std::endl << std::flush; );
    if (!poll(time)) {
        std::cerr << "ERROR: Timeout waiting for reply"
                  << std::endl << std::flush;
        regress( std::cout << "Timeout while waiting for reply"
                           << std::endl << std::flush;);
        return true;
    }
    if (!reply_flag) {
        std::cerr << "ERROR: Reply was not decoded as an operation"
                  << std::endl << std::flush;
        regress( std::cout << "Invalid reply received"
                           << std::endl << std::flush;);
        return true;
    }
    // codec->Poll();
    debug(std::cout << "WAIT finished" << std::endl << std::flush;);
    return error_flag;
}

bool ClientConnection::waitFor(const std::string & opParent,
                               const Object::MapType & arg)
{
    if (wait(timeOut)) {
        return true;
    }
    RootOperation * op;
    std::string opP;
    do {
        poll(0);
        op = pop();
        if (op == NULL) {
            std::cerr << "ERROR: No response to operation"
                      << std::endl << std::flush;
            return true;
        }
        opP = op->GetParents().front().AsString();
        verbose( cout << "Got op of type " << opP << std::endl << std::flush;);
    } while (opP != opParent);
    //const std::string & p = op->GetParents().front().AsString();
    //if (p != opParent) {
        //std::cerr << "ERROR: Response to operation has parent " << p
                  //<< "but it should have parent " << opParent
                  //<< std::endl << std::flush;
        //return true;
    //}
    const Object::ListType & args = op->GetArgs();
    if (arg.empty()) {
        if (!args.empty()) {
            std::cerr << "ERROR: Response to operation has args "
                      << "but no args expected"
                      << std::endl << std::flush;
            return true;
        }
        debug(std::cout << "No arg expected, and none given"
                        << std::endl << std::flush;);
        return false;
    } else {
        if (args.empty()) {
            std::cerr << "ERROR: Response to operation has no args "
                      << "but args are expected"
                      << std::endl << std::flush;
            return true;
        }
        debug(std::cout << "Arg expected, and provided" << std::endl
                        << std::flush;);
    }
    const Object::MapType & a = args.front().AsMap();
    Object::MapType::const_iterator I, J;
    bool error = false;
    for (I = arg.begin(); I != arg.end(); I++) {
        J = a.find(I->first);
        if (J == a.end()) {
            std::cerr << "ERROR: Response to operation args should have "
                      << "attribute '" << I->first << "' of type "
                      << typeAsString(I->second) << " but it is missing"
                      << std::endl << std::flush;
            error = true;
            continue;
        }
        if (I->second.IsNone()) {
            continue;
        }
        if (I->second.GetType() != J->second.GetType()) {
            if (I->second.IsNum() && J->second.IsNum()) {
                std::cerr << "WARNING: Response to operation args should have "
                          << "attribute '" << I->first << "' of type "
                          << typeAsString(I->second) << " but it is of type "
                          << typeAsString(J->second)
                          << std::endl << std::flush;
            } else {
                std::cerr << "ERROR: Response to operation args should have "
                          << "attribute '" << I->first << "' of type "
                          << typeAsString(I->second) << " but it is of type "
                          << typeAsString(J->second)
                          << std::endl << std::flush;
                error = true;
            }
        }
    }
    return error;
}

void ClientConnection::send(Atlas::Objects::Operation::RootOperation & op)
{
    op.SetSerialno(++serialNo);
    encoder->StreamMessage(&op);
    ios << flush;
}

void ClientConnection::error(const std::string & message)
{
    // FIXME Need operation based error function
}

bool ClientConnection::poll(int time)
{
    fd_set infds;
    struct timeval tv;

    FD_ZERO(&infds);

    FD_SET(client_fd, &infds);

    tv.tv_sec = time;
    tv.tv_usec = 0;

    int retval = select(client_fd+1, &infds, NULL, NULL, &tv);

    if ((retval > 0) && (FD_ISSET(client_fd, &infds))) {
        if (ios.peek() == -1) {
            return false;
        }
        codec->Poll();
        return true;
    }
    return false;

}

RootOperation * ClientConnection::pop()
{
    poll(0);
    if (operationQueue.empty()) {
        return NULL;
    }
    RootOperation * op = operationQueue.front();
    operationQueue.pop_front();
    return op;
}

bool ClientConnection::pending()
{
    return !operationQueue.empty();
}

template<class O>
void ClientConnection::push(const O & op)
{
    reply_flag = true;
    RootOperation * new_op = new O(op); 
    operationQueue.push_back(new_op);
}
