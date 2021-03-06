// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2000 Alistair Riddoch

#include "pythonInterface.h"
#include "ClientConnection.h"

#include <iostream>

#include <stdio.h>
#include <unistd.h>

using Atlas::Message::Element;
using Atlas::Message::ListType;
using Atlas::Message::MapType;
using Atlas::Objects::Root;

/*
 * Beginning of Operation section.
 *
 * This is a python type that wraps up operation objects from
 * Atlas::Objects::Operation namespace.
 *
 */

/*
 * Beginning of Operation methods section.
 */

static PyObject * Operation_setFrom(PyOperation * self, PyObject * args)
{
    // Takes string, returns none
    char * from;
    if (!PyArg_ParseTuple(args, "s", &from)) {
        PyErr_SetString(PyExc_TypeError,"from not a string");
        return NULL;
    }
    self->operation->setFrom(from);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * Operation_setTo(PyOperation * self, PyObject * args)
{
    // Takes string, returns none
    char * to;
    if (!PyArg_ParseTuple(args, "s", &to)) {
        PyErr_SetString(PyExc_TypeError,"to not a string");
        return NULL;
    }
    self->operation->setTo(to);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * Operation_setArgs(PyOperation * self, PyObject * args)
{
    // Takes List, returns none
    PyObject * args_object;
    if (!PyArg_ParseTuple(args, "O", &args_object)) {
        PyErr_SetString(PyExc_TypeError,"args not an object");
        return NULL;
    }
    if (!PyList_Check(args_object)) {
        PyErr_SetString(PyExc_TypeError,"args not a list");
        return NULL;
    }
    ListType argslist;
    for(int i = 0; i < PyList_Size(args_object); i++) {
        PyMessageElement * item = (PyMessageElement *)PyList_GetItem(args_object, i);
        if (!PyMessageElement_Check(item)) {
            PyErr_SetString(PyExc_TypeError,"args contains non Atlas Object");
            return NULL;
        }
        
        argslist.push_back(*(item->m_obj));
    }
    self->operation->setArgsAsList(argslist);

    Py_INCREF(Py_None);
    return Py_None;
}

/*
 * Operation methods structure.
 *
 * Generated from a macro in case we need one for each type of operation.
 *
 */

PyMethodDef RootOperation_methods[] = {
    {"setFrom",         (PyCFunction)Operation_setFrom,         METH_VARARGS},
    {"setTo",           (PyCFunction)Operation_setTo,           METH_VARARGS},
    {"setArgs",         (PyCFunction)Operation_setArgs,         METH_VARARGS},
    {NULL,          NULL}
};


/*
 * Beginning of Operation standard methods section.
 */

static void Operation_dealloc(PyOperation *self)
{
    self->operation.~RootOperation();
    PyMem_DEL(self);
}

static PyObject * Operation_getattr(PyOperation * self, char * name)
{
    return Py_FindMethod(RootOperation_methods, (PyObject *)self, name);
}

PyTypeObject PyOperation_Type = {
        PyObject_HEAD_INIT(&PyType_Type)
        0,                                      // ob_size
        "Operation",                            // tp_name
        sizeof(PyOperation),            // tp_basicsize
        0,                                      // tp_itemsize
        //  methods 
        (destructor)Operation_dealloc,          // tp_dealloc
        0,                                      // tp_print
        (getattrfunc)Operation_getattr,         // tp_getattr
        0,                                      // tp_setattr
        0,                                      // tp_compare
        0,                                      // tp_repr
        0,                                      // tp_as_number
        0,                                      // tp_as_sequence
        0,                                      // tp_as_mapping
        0,                                      // tp_hash
};

/*
 * Beginning of Operation creation functions section.
 */

PyOperation * newPyOperation()
{
    PyOperation * self;
    self = PyObject_NEW(PyOperation, &PyOperation_Type);
    if (self == NULL) {
        return NULL;
    }
    new (&(self->operation)) RootOperation;
    self->operation = NULL;
    return self;
}

static PyMethodDef MessageElement_methods[] = {
        {NULL,          NULL}           /* sentinel */
};

/*
 * Beginning of Object standard methods section.
 */

static void MessageElement_dealloc(PyMessageElement *self)
{
    if (self->m_obj != NULL) {
        delete self->m_obj;
    }
    Py_XDECREF(self->m_attr);
    PyMem_DEL(self);
}

static PyObject * MessageElement_getattr(PyMessageElement *self, char *name)
{
    return Py_FindMethod(MessageElement_methods, (PyObject *)self, name);
}

static int MessageElement_setattr( PyMessageElement *self, char *name, PyObject *v)
{
    if (self->m_obj == NULL) {
        PyErr_SetString(PyExc_TypeError,"invalid object");
        return -1;
    }
    if (self->m_obj->isMap()) {
        MapType & omap = self->m_obj->asMap();
        Element v_obj = PyObject_asMessageElement(v);
        if (v_obj.getType() != Element::TYPE_NONE) {
            omap[name] = v_obj;
            return 0;
        }
    }
    if (self->m_attr == NULL) {
        self->m_attr = PyDict_New();
        if (self->m_attr == NULL) {
            return -1;
        }
    }
    return PyDict_SetItemString(self->m_attr, name, v);
}

PyTypeObject PyMessageElement_Type = {
        PyObject_HEAD_INIT(&PyType_Type)
        0,                                      /*ob_size*/
        "MessageElement",                       /*tp_name*/
        sizeof(PyMessageElement),               /*tp_basicsize*/
        0,                                      /*tp_itemsize*/
        /* methods */
        (destructor)MessageElement_dealloc,     /*tp_dealloc*/
        0,                                      /*tp_print*/
        (getattrfunc)MessageElement_getattr,    /*tp_getattr*/
        (setattrfunc)MessageElement_setattr,    /*tp_setattr*/
        0,                                      /*tp_compare*/
        0,                                      /*tp_repr*/
        0,                                      /*tp_as_number*/
        0,                                      /*tp_as_sequence*/
        0,                                      /*tp_as_mapping*/
        0,                                      /*tp_hash*/
};

/*
 * Beginning of Object creation functions section.
 */

PyMessageElement * newPyMessageElement(PyObject *arg)
{
        PyMessageElement * self;
        self = PyObject_NEW(PyMessageElement, &PyMessageElement_Type);
        if (self == NULL) {
                return NULL;
        }
        self->m_attr = NULL;
        return self;
}

/*
 * Utility functions to munge between Object related types and python types
 */

static PyObject * MapType_asPyObject(const MapType & map)
{
    PyObject * args_pydict = PyDict_New();
    MapType::const_iterator I;
    PyMessageElement * item;
    for(I=map.begin();I!=map.end();I++) {
        const std::string & key = I->first;
        item = newPyMessageElement(NULL);
        if (item == NULL) {
            PyErr_SetString(PyExc_TypeError,"error creating map");
            return NULL;
        }
        item->m_obj = new Element(I->second);
        // PyDict_SetItem() does not eat the reference passed to it
        PyDict_SetItemString(args_pydict,(char *)key.c_str(),(PyObject *)item);
        Py_DECREF(item);
    }
    return args_pydict;
}

static PyObject * ListType_asPyObject(const ListType & list)
{
    PyObject * args_pylist = PyList_New(list.size());
    ListType::const_iterator I;
    int j=0;
    PyMessageElement * item;
    for(I=list.begin();I!=list.end();I++,j++) {
        item = newPyMessageElement(NULL);
        if (item == NULL) {
            PyErr_SetString(PyExc_TypeError,"error creating list");
            return NULL;
        }
        item->m_obj = new Element(*I);
        // PyList_SetItem() eats the reference passed to it
        PyList_SetItem(args_pylist, j, (PyObject *)item);
    }
    return args_pylist;
}

PyObject * MessageElement_asPyObject(const Element & obj)
{
    PyObject * ret = NULL;
    switch (obj.getType()) {
        case Element::TYPE_INT:
            ret = PyInt_FromLong(obj.asInt());
            break;
        case Element::TYPE_FLOAT:
            ret = PyFloat_FromDouble(obj.asFloat());
            break;
        case Element::TYPE_STRING:
            ret = PyString_FromString(obj.asString().c_str());
            break;
        case Element::TYPE_MAP:
            ret = MapType_asPyObject(obj.asMap());
            break;
        case Element::TYPE_LIST:
            ret = ListType_asPyObject(obj.asList());
            break;
        default:
            break;
    }
    return ret;
}

ListType PyListObject_asElementList(PyObject * list)
{
    ListType argslist;
    PyMessageElement * item;
    for(int i = 0; i < PyList_Size(list); i++) {
        item = (PyMessageElement *)PyList_GetItem(list, i);
        if (PyMessageElement_Check(item)) {
            argslist.push_back(*(item->m_obj));
        } else {
            Element o = PyObject_asMessageElement((PyObject*)item);
            if (o.getType() != Element::TYPE_NONE) {
                argslist.push_back(o);
            }
        }
    }
    return argslist;
}

MapType PyDictObject_asElementMap(PyObject * dict)
{
    MapType argsmap;
    PyMessageElement * item;
    PyObject * keys = PyDict_Keys(dict);
    PyObject * vals = PyDict_Values(dict);
    for(int i = 0; i < PyDict_Size(dict); i++) {
        PyObject * key = PyList_GetItem(keys, i);
        item = (PyMessageElement *)PyList_GetItem(vals, i);
        if (PyMessageElement_Check(item)) {
            argsmap[PyString_AsString(key)] = *(item->m_obj);
        } else {
            Element o = PyObject_asMessageElement((PyObject*)item);
            if (o.getType() != Element::TYPE_NONE) {
                argsmap[PyString_AsString(key)] = o;
            }
        }
    }
    Py_DECREF(keys);
    Py_DECREF(vals);
    return argsmap;
}

Element PyObject_asMessageElement(PyObject * o)
{
    if (PyInt_Check(o)) {
        return Element((int)PyInt_AsLong(o));
    }
    if (PyFloat_Check(o)) {
        return Element(PyFloat_AsDouble(o));
    }
    if (PyString_Check(o)) {
        return Element(PyString_AsString(o));
    }
    if (PyList_Check(o)) {
        return Element(PyListObject_asElementList(o));
    }
    if (PyDict_Check(o)) {
        return Element(PyDictObject_asElementMap(o));
    }
    if (PyTuple_Check(o)) {
        ListType list;
        int i, size = PyTuple_Size(o);
        for(i = 0; i < size; i++) {
            Element item = PyObject_asMessageElement(PyTuple_GetItem(o, i));
            if (item.getType() != Element::TYPE_NONE) {
                list.push_back(item);
            }
        }
        return Element(list);
    }
    if (PyMessageElement_Check(o)) {
        PyMessageElement * obj = (PyMessageElement *)o;
        return *(obj->m_obj);
    }
    if (PyOperation_Check(o)) {
        PyOperation * op = (PyOperation *)o;
        return op->operation->asMessage();
    }
    return Element();
}

static PyObject * PyConnection_connect(PyConnection * self, PyObject * args)
{
    if (self->connection == NULL) {
        PyErr_SetString(PyExc_TypeError,"invalid connection");
        return NULL;
    }
    char * host;
    if (!PyArg_ParseTuple(args, "s", &host)) {
        PyErr_SetString(PyExc_TypeError,"send takes one argument");
        return NULL;
    }
    bool ret = self->connection->connect(host);
    if (ret) {
        Py_INCREF(Py_True);
        return Py_True;
    } else {
        Py_INCREF(Py_False);
        return Py_False;
    }
}

static PyObject * PyConnection_login(PyConnection * self, PyObject * args)
{
    if (self->connection == NULL) {
        PyErr_SetString(PyExc_TypeError,"invalid connection");
        return NULL;
    }
    char * name;
    char * passwd;
    if (!PyArg_ParseTuple(args, "ss", &name, &passwd)) {
        PyErr_SetString(PyExc_TypeError,"login takes two string arguments");
        return NULL;
    }
    bool ret = self->connection->login(name, passwd);
    if (ret) {
        Py_INCREF(Py_True);
        return Py_True;
    } else {
        Py_INCREF(Py_False);
        return Py_False;
    }
}

static PyObject * PyConnection_create(PyConnection * self, PyObject * args)
{
    if (self->connection == NULL) {
        PyErr_SetString(PyExc_TypeError,"invalid connection");
        return NULL;
    }
    char * name;
    char * passwd;
    if (!PyArg_ParseTuple(args, "ss", &name, &passwd)) {
        PyErr_SetString(PyExc_TypeError,"create takes two string arguments");
        return NULL;
    }
    bool ret = self->connection->create(name, passwd);
    if (ret) {
        Py_INCREF(Py_True);
        return Py_True;
    } else {
        Py_INCREF(Py_False);
        return Py_False;
    }
}

static PyObject * PyConnection_send(PyConnection * self, PyObject * args)
{
    if (self->connection == NULL) {
        PyErr_SetString(PyExc_TypeError,"invalid connection");
        return NULL;
    }
    PyOperation * op;
    if (!PyArg_ParseTuple(args, "O", &op)) {
        PyErr_SetString(PyExc_TypeError,"send takes one argument");
        return NULL;
    }
    if (!PyOperation_Check(op)) {
        PyErr_SetString(PyExc_TypeError,"send argument must be an op");
        return NULL;
    }
    self->connection->send(op->operation);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * PyConnection_waitfor(PyConnection * self, PyObject * args)
{
    if (self->connection == NULL) {
        PyErr_SetString(PyExc_TypeError,"invalid connection");
        return NULL;
    }
    char * parents;
    PyMessageElement * arg;
    if (!PyArg_ParseTuple(args, "sO", &parents, &arg)) {
        PyErr_SetString(PyExc_TypeError,"waitfor takes two arguments");
        return NULL;
    }
    if (!PyMessageElement_Check(arg)) {
        PyErr_SetString(PyExc_TypeError,"waitfor argument must be an entity");
        return NULL;
    }
    bool ret = self->connection->waitFor(parents, arg->m_obj->asMap());
    if (ret) {
        Py_INCREF(Py_True);
        return Py_True;
    } else {
        Py_INCREF(Py_False);
        return Py_False;
    }
}

PyMethodDef PyConnection_methods[] = {
    {"connect",    (PyCFunction)PyConnection_connect,      METH_VARARGS},
    {"login",      (PyCFunction)PyConnection_login,        METH_VARARGS},
    {"create",     (PyCFunction)PyConnection_create,       METH_VARARGS},
    {"send",       (PyCFunction)PyConnection_send,         METH_VARARGS},
    {"waitfor",    (PyCFunction)PyConnection_waitfor,      METH_VARARGS},
    {NULL,         NULL}
};

static void PyConnection_dealloc(PyConnection *self)
{
    PyMem_DEL(self);
}

static PyObject * PyConnection_getattr(PyConnection *self, char *name)
{
    std::cout << "getattr: " << name << std::endl << std::flush;
    return Py_FindMethod(PyConnection_methods, (PyObject *)self, name);
}

PyTypeObject PyConnection_Type = {
        PyObject_HEAD_INIT(&PyType_Type)
        0,                              /*ob_size*/
        "PyConnection",             /*tp_name*/
        sizeof(PyConnection),       /*tp_basicsize*/
        0,                              /*tp_itemsize*/
        /* methods */
        (destructor)PyConnection_dealloc, /*tp_dealloc*/
        0,                              /*tp_print*/
        (getattrfunc)PyConnection_getattr,/*tp_getattr*/
        0,                              /*tp_setattr*/
        0,                              /*tp_compare*/
        0,                              /*tp_repr*/
        0,                              /*tp_as_number*/
        0,                              /*tp_as_sequence*/
        0,                              /*tp_as_mapping*/
        0,                              /*tp_hash*/
};

PyConnection * newPyConnection()
{
    PyConnection * self;
    self = PyObject_NEW(PyConnection, &PyConnection_Type);
    if (self == NULL) {
        return NULL;
    }
    return self;
}

static PyObject * entity_new(PyObject * self, PyObject * args, PyObject * kwds)
{
    PyMessageElement *o;
    char * id = NULL;
    
    if (!PyArg_ParseTuple(args, "|s", &id)) {
        return NULL;
    }
    MapType omap;
    if (id != NULL) {
        omap["id"] = std::string(id);
    }
    if ((kwds != NULL) && (PyDict_Check(kwds))) {
        PyObject * keys = PyDict_Keys(kwds);
        PyObject * vals = PyDict_Values(kwds);
        if ((keys == NULL) || (vals == NULL)) {
            PyErr_SetString(PyExc_TypeError, "Error in keywords");
            return NULL;
        }
        int i, size=PyList_Size(keys); 
        for(i = 0; i < size; i++) {
            char * key = PyString_AsString(PyList_GetItem(keys, i));
            PyObject * val = PyList_GetItem(vals, i);
            if ((strcmp(key, "parent") == 0) && (PyString_Check(val))) {
                omap["loc"] = Element(std::string(PyString_AsString(val)));
            } else if ((strcmp(key, "type") == 0) && (PyString_Check(val))) {
                omap["parents"] = ListType(1,std::string(PyString_AsString(val)));
            } else {
                Element val_obj = PyObject_asMessageElement(val);
                if (val_obj.getType() == Element::TYPE_NONE) {
                    fprintf(stderr, "Could not handle %s value in Entity()", key);
                    PyErr_SetString(PyExc_TypeError, "Argument type error to Entity()");
                    Py_DECREF(keys);
                    Py_DECREF(vals);
                    return NULL;
                }
                omap[key] = val_obj;
            }
        }
        Py_DECREF(keys);
        Py_DECREF(vals);
    }

    o = newPyMessageElement(args);
    if ( o == NULL ) {
        return NULL;
    }
    o->m_obj = new Element(omap);
    return (PyObject *)o;
}

static int addToArgs(std::vector<Root> & args, PyObject * arg)
{
    if (PyMessageElement_Check(arg)) {
        PyMessageElement * obj = (PyMessageElement*)arg;
#ifndef NDEBUG
        if (obj->m_obj == NULL) {
            PyErr_SetString(PyExc_AssertionError,"NULL MessageElement in Operation constructor argument");
            return -1;
        }
#endif // NDEBUG
        const Element & o = *obj->m_obj;
        if (o.isMap()) {
            args.push_back(Atlas::Objects::Factories::instance()->createObject(o.asMap()));
        } else {
            PyErr_SetString(PyExc_TypeError, "Operation arg is not a map");
            return -1;
        }
    } else if (PyOperation_Check(arg)) {
        PyOperation * op = (PyOperation*)arg;
#ifndef NDEBUG
        if (!op->operation.isValid()) {
            PyErr_SetString(PyExc_AssertionError,"Invalid operation in Operation constructor argument");
            return -1;
        }
#endif // NDEBUG
        args.push_back(op->operation);
    } else {
        PyErr_SetString(PyExc_TypeError, "Operation arg is of unknown type");
        return -1;
    }
    return 0;
}

static PyObject * operation_new(PyObject * self, PyObject * args, PyObject * kwds)
{
    PyOperation * op;

    char * type;
    PyObject * arg1 = NULL;
    PyObject * arg2 = NULL;
    PyObject * arg3 = NULL;

    if (!PyArg_ParseTuple(args, "s|OOO", &type, &arg1, &arg2, &arg3)) {
        return NULL;
    }
    op = newPyOperation();
    if (op == NULL) {
        return NULL;
    }
    if (strcmp(type, "thought") == 0 || strcmp(type, "goal_info") == 0) {
        Py_DECREF(op);
        Py_INCREF(Py_None);
        return Py_None;
    } else {
        Root r = Atlas::Objects::Factories::instance()->createObject(type);
        op->operation = Atlas::Objects::smart_dynamic_cast<RootOperation>(r);
        if (!op->operation.isValid()) {
            Py_DECREF(op);
            PyErr_SetString(PyExc_TypeError, "Operation() unknown operation type requested");
            return NULL;
        }
    }
    if (kwds != NULL) {
        PyObject * from = PyDict_GetItemString(kwds, "from_");
        if (from != NULL) {
            PyObject * from_id = 0;
            if (PyString_Check(from)) {
                from_id = from;
                Py_INCREF(from_id);
            } else if ((from_id = PyObject_GetAttrString(from, "id")) == NULL) {
                PyErr_SetString(PyExc_TypeError, "from is not a string and has no id");
                return NULL;
            }
            if (!PyString_Check(from_id)) {
                Py_DECREF(from_id);
                PyErr_SetString(PyExc_TypeError, "id of from is not a string");
                return NULL;
            }
            op->operation->setFrom(PyString_AsString(from_id));
            Py_DECREF(from_id);
        }
        PyObject * to = PyDict_GetItemString(kwds, "to");
        if (to != NULL) {
            PyObject * to_id = 0;
            if (PyString_Check(to)) {
                to_id = to;
                Py_INCREF(to_id);
            } else if ((to_id = PyObject_GetAttrString(to, "id")) == NULL) {
                PyErr_SetString(PyExc_TypeError, "to is not a string and has no id");
                return NULL;
            }
            if (!PyString_Check(to_id)) {
                Py_DECREF(to_id);
                PyErr_SetString(PyExc_TypeError, "id of to is not a string");
                return NULL;
            }
            op->operation->setTo(PyString_AsString(to_id));
            Py_DECREF(to_id);
        }
    }
    std::vector<Root> & args_list = op->operation->modifyArgs();
    assert(args_list.empty());
    if (arg1 != 0 && addToArgs(args_list, arg1) != 0) {
        Py_DECREF(op);
        op = NULL;
    }
    if (arg2 != 0 && addToArgs(args_list, arg2) != 0) {
        Py_DECREF(op);
        op = NULL;
    }
    if (arg3 != 0 && addToArgs(args_list, arg3) != 0) {
        Py_DECREF(op);
        op = NULL;
    }
    return (PyObject *)op;
}

static PyObject * connection_new(PyObject* self, PyObject* args)
{
    PyConnection *c;
    if (!PyArg_ParseTuple(args, "")) {
        return NULL;
    }
    c = newPyConnection();
    if (c == NULL) {
        return NULL;
    }
    c->connection = new ClientConnection();
    return (PyObject *)c;
}

static PyMethodDef atlas_methods[] = {
    {"Operation",  (PyCFunction)operation_new,  METH_VARARGS|METH_KEYWORDS},
    {"Entity",     (PyCFunction)entity_new,     METH_VARARGS|METH_KEYWORDS},
    {"Connection", (PyCFunction)connection_new, METH_VARARGS},
    {NULL,              NULL}                           /* Sentinel */
};

void init_python_api()
{
    std::string pypath(".");

    setenv("PYTHONPATH", pypath.c_str(), 1);

    Py_Initialize();

    if (Py_InitModule("atlas", atlas_methods) == NULL) {
        fprintf(stderr, "Failed to Create atlas module\n");
        return;
    }
}

bool runScript(const std::string & script,
               const std::set<ClientConnection *> & connections)
{
    PyObject * package_name = PyString_FromString(script.c_str());
    PyObject * mod_dict = PyImport_Import(package_name);
    Py_DECREF(package_name);
    if (mod_dict == NULL) {
        std::cerr << "Cld not find python module " << script
                  << std::endl << std::flush;
        PyErr_Print();
        return false;
    }
    PyObject * function = PyObject_GetAttrString(mod_dict, "process");
    Py_DECREF(mod_dict);
    if (function == NULL) {
        std::cerr << "Could not find process function" << std::endl
                  << std::flush;
        PyErr_Print();
        return false;
    }
    if (PyCallable_Check(function) == 0) {
        std::cerr << "It does not seem to be a function at all" << std::endl
                  << std::flush;
        Py_DECREF(function);
        return false;
    }
    PyObject * pyob;
    if (connections.empty()) {
        pyob = PyEval_CallFunction(function, "()");
    } else {
        PyObject * cons = PyTuple_New(connections.size());
        std::set<ClientConnection *>::const_iterator I = connections.begin();
        for(int i = 0; I != connections.end(); ++I, ++i) {
            PyConnection * connection = newPyConnection();
            connection->connection = *I;
            PyTuple_SetItem(cons, i, (PyObject*)connection);
        }
        pyob = PyEval_CallFunction(function, "(O)", cons);
        Py_DECREF(cons);
    }

    if (pyob == NULL) {
        if (PyErr_Occurred() == NULL) {
            std::cerr << "Could not call function" << std::endl << std::flush;
        } else {
            std::cerr << "Reporting python error" << std::endl << std::flush;
            PyErr_Print();
        }
    }
    Py_DECREF(function);
    return true;
}
