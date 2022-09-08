// This is implemented as a TPP header file to avoid building this code unless
// it's used. This is done for efficiency as a board will only have one or two
// different CAN controllers.

namespace Canny {
namespace {

Bitrate FixMCP2515Bitrate(Bitrate bitrate) {
    switch (bitrate) {
        case CAN20_125K:
        case CANFD_125K:
        case CANFD_125K_500K:
            return CAN20_125K;
        case CAN20_250K:
        case CANFD_250K:
        case CANFD_250K_500K:
        case CANFD_250K_750K:
        case CANFD_250K_1M:
        case CANFD_250K_1M5:
        case CANFD_250K_2M:
        case CANFD_250K_3M:
        case CANFD_250K_4M:
            return CAN20_250K;
        case CAN20_500K:
        case CANFD_500K:
        case CANFD_500K_1M:
        case CANFD_500K_2M:
        case CANFD_500K_3M:
        case CANFD_500K_4M:
        case CANFD_500K_5M:
        case CANFD_500K_6M5:
        case CANFD_500K_8M:
        case CANFD_500K_10M:
            return CAN20_500K;
        case CAN20_1000K:
        case CANFD_1000K:
        case CANFD_1000K_4M:
        case CANFD_1000K_8M:
            return CAN20_1000K;
    }
    return CAN20_250K;
} 

uint8_t GetMCP2515Bitrate(Bitrate bitrate) {
    switch (bitrate) {
        case CAN20_125K:
            return CAN_125KBPS;
        default:
        case CAN20_250K:
            return CAN_250KBPS;
        case CAN20_500K:
            return CAN_500KBPS;
        case CAN20_1000K:
            return CAN_1000KBPS;
    }
}

}  // namespace

bool MCP2515::begin(Bitrate bitrate) {
    bitrate_ = FixMCP2515Bitrate(bitrate);
    ready_ = mcp_.begin(GetMCP2515Bitrate(bitrate_)) == CAN_OK;
    return ready_;
}

Mode MCP2515::mode() const {
    return CAN20;
}

Bitrate MCP2515::bitrate() const {
    return bitrate_;
}

Error MCP2515::read(Frame* frame) {
    if (!ready_) {
        return ERR_READY;
    }
    frame->reserve(8);

    switch (mcp_.readMsgBufID(frame->mutable_id(), frame->mutable_size(), frame->data())) {
        case CAN_OK:
            break;
        case CAN_NOMSG:
            return ERR_FIFO;
        default:
            return ERR_INTERNAL;
    }
    frame->ext(mcp_.isExtendedFrame());
    return ERR_OK;
}

Error MCP2515::write(const Frame& frame) {
    if (!ready_) {
        return ERR_READY;
    }
    if (frame.data() == nullptr || frame.size() > 8) {
        return ERR_INVALID;
    }

    switch(mcp_.sendMsgBuf(frame.id(), frame.ext(), frame.size(), frame.data())) {
        case CAN_OK:
            return ERR_OK;
        case CAN_GETTXBFTIMEOUT:
        case CAN_SENDMSGTIMEOUT:
            return ERR_FIFO;
        default:
            return ERR_INTERNAL;
    }
}

}  // namespace Canny
