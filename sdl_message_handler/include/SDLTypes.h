
#ifndef _SDL_MESSAGE_HANDLER_TYPES_
#define _SDL_MESSAGE_HANDLER_TYPES_


namespace sdlcore_message_handler {

    enum ErrorCode {
        CONTROLLER_EXISTS = -32000,
        SUBSCRIBTION_EXISTS = -32001,
        INVALID_REQUEST = -32600
    };

    enum class ListType : uint32_t {
        CONTACT = 0,
        CALL_LOG,
        SMS
    };

    enum class CallLogType : uint32_t {
        IN_CALL = 1,
        DIAL,
        MISSED
    };
}

#endif  // _SDL_MESSAGE_HANDLER_TYPES_
