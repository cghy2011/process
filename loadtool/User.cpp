#include "User.h"
#include "Character.h"

#include <sigc++/object_slot.h>
#include <Atlas/Objects/Entity.h>
#include <iostream>

using std::cout;
using std::endl;
using std::cerr;

User::User(const std::string& uid, const std::string& host, short port) :
    m_state(CONNECTING),
    m_uid(uid),
    m_acc(NULL),
    m_charCount(1)
{
    cout << "creating user " << uid << endl;
    m_con = new Eris::Connection("fuckwad", false);
    m_con->Connected.connect(SigC::slot(*this, &User::onConnected));
    m_con->Failure.connect(SigC::slot(*this, &User::onConnectionFail));
    m_con->connect(host, port);
}

User::~User()
{
    delete m_acc;
    delete m_con;
}

void User::createCharacter()
{
    Atlas::Objects::Entity::GameEntity charData;
    m_acc->createCharacter(charData);
}

#pragma mark -

void User::onConnected()
{
    cout << "user connected to server" << endl;
    
    m_acc = new Eris::Account(m_con);
    
    std::string fullName = "Fuckwad User";
    std::string username = m_uid;
    std::string passwd = "secret-romero-love-hideout";
     
    m_acc->createAccount(username, fullName, passwd);
    m_acc->LoginSuccess.connect(SigC::slot(*this, &User::onLoggedIn));
    m_acc->LoginFailure.connect(SigC::slot(*this, &User::onLoginFailure));
    m_acc->AvatarSuccess.connect(SigC::slot(*this, &User::onCharCreated));
    
    m_state = CONNECTED;
}

void User::onConnectionFail(const std::string& errMsg)
{
    cerr << "User " << m_uid << " got Eris::Connection failure: " << errMsg << endl;
    m_state = FAILED;
}

void User::onLoggedIn()
{
    if (m_charCount) createCharacter();
}

void User::onLoginFailure(const std::string& errMsg)
{
    cerr << "User " << m_uid << " got account login failure: " << errMsg << endl;
    m_state = FAILED;
}

void User::onCharCreated(Eris::Avatar* av)
{
    Character* c = new Character(av);
    if (--m_charCount) createCharacter();
    m_chars[av->getId()] = c;
}

