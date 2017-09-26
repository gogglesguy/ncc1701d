#ifndef GAP_EXCEPTIONS_H
#define GAP_EXCEPTIONS_H

namespace gap {

/* Base class for all exceptions */
class exception {};

namespace exceptions {

// Resource Exception
class resource : public gap::exception {};

class unimplemented : public gap::exception {};

// Input/Output Exception
class io : public gap::exception {};
class data : public  gap::exceptions::io {};
class position : public  gap::exceptions::io {};
class eof : public gap::exceptions::io {};

}
}

#endif
