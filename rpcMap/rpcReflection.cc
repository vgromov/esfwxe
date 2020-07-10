// core definitions of rpc reflection engine
RPC_REFLECTION_BEGIN
    RPC_DEF_HANDLER0_NORETURN()
    RPC_DEF_HANDLER0(esU8)
    RPC_DEF_HANDLER0(esU16)
    RPC_DEF_HANDLER0(esU32)
    RPC_DEF_HANDLER0(esU64)
    RPC_DEF_HANDLER0(esI8)
    RPC_DEF_HANDLER0(esI16)
    RPC_DEF_HANDLER0(esI32)
    RPC_DEF_HANDLER0(esI64)
    RPC_DEF_HANDLER0(esBL)
    RPC_DEF_HANDLER0(esDT)
    RPC_DEF_HANDLER0(esBA)
    RPC_DEF_HANDLER0(esF)
    RPC_DEF_HANDLER1_NORETURN(    esBA)
    RPC_DEF_HANDLER1_NORETURN(    esU16)
    RPC_DEF_HANDLER1_NORETURN(    esF)
    RPC_DEF_HANDLER1(esBL,        esDT)
    RPC_DEF_HANDLER1(esBA,        esU16)
    RPC_DEF_HANDLER1_NORETURN(    esU8)
    RPC_DEF_HANDLER1_NORETURN(    esU32)
    RPC_DEF_HANDLER1_NORETURN(    esBL)
    RPC_DEF_HANDLER2_NORETURN(    esU8,           esF)
    RPC_DEF_HANDLER1(esF,         esU8)
    RPC_DEF_HANDLER1(esU16,       esBA)
    RPC_DEF_HANDLER2(esU16,       esBA,           esBL)
    RPC_DEF_HANDLER1(esBA,        esBA)
    RPC_DEF_HANDLER2_NORETURN(    esU8,           esBA)
    RPC_DEF_HANDLER1(esBA,        esU32)
    RPC_DEF_HANDLER2(esBA,        esU32,          esU32)
    RPC_DEF_HANDLER2_NORETURN(    esU32,          esBA)
    RPC_DEF_HANDLER1(esBA,        esU8)
    RPC_DEF_HANDLER3_NORETURN(    esU8,           esU8,       esU16)
    RPC_DEF_HANDLER3_NORETURN(    esU32,          esU32,      esBA)
    RPC_DEF_HANDLER4_NORETURN(    esU32,          esU32,      esU32,    esBA)
    RPC_DEF_HANDLER2(esU32,       esU32,          esBA)
RPC_REFLECTION_END

// macro defines housekeeping
#undef RPC_REFLECTION_BEGIN
#undef RPC_DEF_HANDLER0_NORETURN
#undef RPC_DEF_HANDLER1_NORETURN
#undef RPC_DEF_HANDLER2_NORETURN
#undef RPC_DEF_HANDLER3_NORETURN
#undef RPC_DEF_HANDLER4_NORETURN
#undef RPC_DEF_HANDLER5_NORETURN
#undef RPC_DEF_HANDLER6_NORETURN
#undef RPC_DEF_HANDLER7_NORETURN
#undef RPC_DEF_HANDLER0
#undef RPC_DEF_HANDLER1
#undef RPC_DEF_HANDLER2
#undef RPC_DEF_HANDLER3
#undef RPC_DEF_HANDLER4
#undef RPC_DEF_HANDLER5
#undef RPC_DEF_HANDLER6
#undef RPC_DEF_HANDLER7
#undef RPC_REFLECTION_END
