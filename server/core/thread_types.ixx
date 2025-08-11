export module thread.types;

export enum class ThreadType : uint16_t {
    Unknown = 0,
    Logic,
    Database,
    Gateway,
    Network,
};
