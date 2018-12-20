#ifndef BYTECODE_RUNNER_H
#define BYTECODE_RUNNER_H

#include <stdint.h>
#include <stdbool.h>

enum bytecode_register_kind
{
    BYTECODE_REGISTER_KIND_I64 = 0,
    BYTECODE_REGISTER_KIND_I32 = 1,
    BYTECODE_REGISTER_KIND_I16 = 2,
    BYTECODE_REGISTER_KIND_I8  = 3,
    BYTECODE_REGISTER_KIND_F64 = 4,
    BYTECODE_REGISTER_KIND_F32 = 5,
    BYTECODE_REGISTER_KIND_NON = 6,
};

static const char *bytecode_register_str[] =
{
    "RIP",

    "RAX",
    "RBX",
    "RCX",
    "RDX",
    "RBP",
    "RSP",
    "RSI",
    "RDI",

    "R8",
    "R9",
    "R10",
    "R11",
    "R12",
    "R13",
    "R14",
    "R15",

    "COUNT"
};

enum bytecode_register
{
    BYTECODE_REGISTER_RIP,

    BYTECODE_REGISTER_RAX,
    BYTECODE_REGISTER_RBX,
    BYTECODE_REGISTER_RCX,
    BYTECODE_REGISTER_RDX,
    BYTECODE_REGISTER_RBP,
    BYTECODE_REGISTER_RSP,
    BYTECODE_REGISTER_RSI,
    BYTECODE_REGISTER_RDI,

    BYTECODE_REGISTER_R8,
    BYTECODE_REGISTER_R9,
    BYTECODE_REGISTER_R10,
    BYTECODE_REGISTER_R11,
    BYTECODE_REGISTER_R12,
    BYTECODE_REGISTER_R13,
    BYTECODE_REGISTER_R14,
    BYTECODE_REGISTER_R15,

    BYTECODE_REGISTER_COUNT
};

static enum bytecode_register bytecode_call_registers[] =
{
    BYTECODE_REGISTER_RDI,
    BYTECODE_REGISTER_RSI,
    BYTECODE_REGISTER_RDX,
    BYTECODE_REGISTER_RCX,
    BYTECODE_REGISTER_R8,
    BYTECODE_REGISTER_R9,
    BYTECODE_REGISTER_R10,
    BYTECODE_REGISTER_R11,
    BYTECODE_REGISTER_R12,
    BYTECODE_REGISTER_R13,
    BYTECODE_REGISTER_R14,
    BYTECODE_REGISTER_R15,
};

static enum bytecode_register bytecode_internal_call_registers[] =
{
    BYTECODE_REGISTER_RDI,
    BYTECODE_REGISTER_RSI,
    BYTECODE_REGISTER_R8,
    BYTECODE_REGISTER_R10,
    BYTECODE_REGISTER_R11,
    BYTECODE_REGISTER_R12,
    BYTECODE_REGISTER_R13,
    BYTECODE_REGISTER_R14,
    BYTECODE_REGISTER_R15,
};

struct bytecode_runner
{
    bool verbose;
    bool single_step;

    bool is_running;
    uint64_t cycle_count;

    char *data;
    uint64_t data_size;

    uint64_t *text;
    uint64_t text_size;

    uint64_t stack_size;
    char *stack;

    uint64_t reg[BYTECODE_REGISTER_COUNT];
    uint64_t reg_type[BYTECODE_REGISTER_COUNT];
    int compare;
};

struct bytecode_result
{
    enum bytecode_register_kind kind;
    union
    {
        uint8_t i8;
        uint16_t i16;
        uint32_t i32;
        uint64_t i64;

        float f32;
        double f64;
    };
};

struct bytecode_instruction;
struct bytecode_executable;

void bytecode_runner_init(struct bytecode_runner *bcr, struct bytecode_executable *program);
struct bytecode_result bytecode_runner_run(struct bytecode_runner *bcr);
void bytecode_runner_destroy(struct bytecode_runner *bcr);

void bytecode_runner_print_registers(struct bytecode_runner *bcr);
void bytecode_runner_print_stack(struct bytecode_runner *bcr);
void bytecode_runner_print_instruction(struct bytecode_runner *bcr, struct bytecode_instruction *instr);

#endif