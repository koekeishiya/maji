#include "../resolve.h"

#include "../string_util.h"

#include "bytecode_runner.h"
#include "bytecode_opcode.h"

#include "bytecode_instruction.h"
#include "bytecode_instruction.c"

#include "bytecode_executable.h"
#include "bytecode_executable.c"

uint64_t _memw_reg_reg(enum bytecode_register reg1, enum bytecode_register reg2)
{
    return encode_instruction_r2(BYTECODE_OPCODE_MEMW_REG_REG, reg1, reg2);
}

uint64_t _memr_i64_reg_reg(enum bytecode_register reg1, enum bytecode_register reg2)
{
    return encode_instruction_r2(BYTECODE_OPCODE_MEMR_INT64_REG_REG, reg1, reg2);
}

uint64_t _lea_lcl_reg_imm(enum bytecode_register reg)
{
    return encode_instruction_r1(BYTECODE_OPCODE_LEA_LCL_REG_IMM, reg);
}

uint64_t _lea_bss_reg_imm(enum bytecode_register reg)
{
    return encode_instruction_r1(BYTECODE_OPCODE_LEA_BSS_REG_IMM, reg);
}

uint64_t _mov_i64_lcl_imm(void)
{
    return encode_instruction(BYTECODE_OPCODE_MOV_INT64_LCL_IMM);
}

uint64_t _mov_lcl_reg(enum bytecode_register reg)
{
    return encode_instruction_r1(BYTECODE_OPCODE_MOV_LCL_REG, reg);
}

uint64_t _mov_i64_reg_imm(enum bytecode_register reg)
{
    return encode_instruction_r1(BYTECODE_OPCODE_MOV_INT64_REG_IMM, reg);
}

uint64_t _mov_reg_reg(enum bytecode_register reg1, enum bytecode_register reg2)
{
    return encode_instruction_r2(BYTECODE_OPCODE_MOV_REG_REG, reg1, reg2);
}

uint64_t _add_i64_reg_imm(enum bytecode_register reg)
{
    return encode_instruction_r1(BYTECODE_OPCODE_ADD_INT64_REG_IMM, reg);
}

uint64_t _add_reg_reg(enum bytecode_register reg1, enum bytecode_register reg2)
{
    return encode_instruction_r2(BYTECODE_OPCODE_ADD_REG_REG, reg1, reg2);
}

uint64_t _sub_i64_reg_imm(enum bytecode_register reg)
{
    return encode_instruction_r1(BYTECODE_OPCODE_SUB_INT64_REG_IMM, reg);
}

uint64_t _sub_reg_reg(enum bytecode_register reg1, enum bytecode_register reg2)
{
    return encode_instruction_r2(BYTECODE_OPCODE_SUB_REG_REG, reg1, reg2);
}

uint64_t _mul_reg_reg(enum bytecode_register reg1, enum bytecode_register reg2)
{
    return encode_instruction_r2(BYTECODE_OPCODE_MUL_REG_REG, reg1, reg2);
}

uint64_t _div_reg_reg(enum bytecode_register reg1, enum bytecode_register reg2)
{
    return encode_instruction_r2(BYTECODE_OPCODE_DIV_REG_REG, reg1, reg2);
}

uint64_t _push_reg(enum bytecode_register reg)
{
    return encode_instruction_r1(BYTECODE_OPCODE_PUSH_REG, reg);
}

uint64_t _pop_i64_reg(enum bytecode_register reg)
{
    return encode_instruction_r1(BYTECODE_OPCODE_POP_INT64_REG, reg);
}

uint64_t _cmp_reg_reg(enum bytecode_register reg1, enum bytecode_register reg2)
{
    return encode_instruction_r2(BYTECODE_OPCODE_CMP_REG_REG, reg1, reg2);
}

uint64_t _cmp_reg_imm(enum bytecode_register reg)
{
    return encode_instruction_r1(BYTECODE_OPCODE_CMP_REG_IMM, reg);
}

uint64_t _jle_imm(void)
{
    return encode_instruction(BYTECODE_OPCODE_JLE_IMM);
}

uint64_t _jl_imm(void)
{
    return encode_instruction(BYTECODE_OPCODE_JL_IMM);
}

uint64_t _jge_imm(void)
{
    return encode_instruction(BYTECODE_OPCODE_JGE_IMM);
}

uint64_t _jg_imm(void)
{
    return encode_instruction(BYTECODE_OPCODE_JG_IMM);
}

uint64_t _jz_imm(void)
{
    return encode_instruction(BYTECODE_OPCODE_JZ_IMM);
}

uint64_t _jnz_imm(void)
{
    return encode_instruction(BYTECODE_OPCODE_JNZ_IMM);
}

uint64_t _jmp_imm(void)
{
    return encode_instruction(BYTECODE_OPCODE_JMP_IMM);
}

uint64_t _call_reg(enum bytecode_register reg)
{
    return encode_instruction_r1(BYTECODE_OPCODE_CALL_REG, reg);
}

uint64_t _call_imm(void)
{
    return encode_instruction(BYTECODE_OPCODE_CALL_IMM);
}

uint64_t _call_foreign(void)
{
    return encode_instruction(BYTECODE_OPCODE_CALL_FOREIGN);
}

uint64_t _halt(void)
{
    return encode_instruction(BYTECODE_OPCODE_HALT);
}

uint64_t _begin_call_frame(void)
{
    return encode_instruction(BYTECODE_OPCODE_BEGIN_CALL_FRAME);
}

uint64_t _end_call_frame(void)
{
    return encode_instruction(BYTECODE_OPCODE_END_CALL_FRAME);
}

uint64_t _ret(void)
{
    return encode_instruction(BYTECODE_OPCODE_RETURN);
}

struct bytecode_emitter
{
    char *data_cursor;
    char *program_data;
    uint64_t data_size;
    uint64_t *text_cursor;
    uint64_t *program_text;
    uint64_t text_size;
    uint64_t *break_patches[12];
    uint64_t break_patches_count;
    uint64_t *continue_patches[12];
    uint64_t continue_patches_count;
    uint64_t *return_patches[12];
    uint64_t return_patches_count;
    uint64_t **call_patches;
    struct symbol **call_patches_symbol;
    const uint8_t *entry_point;
    bool entry_point_patched;
    struct resolver *resolver;
};

void bytecode_emit_expression(struct bytecode_emitter *emitter, struct ast_expr *expr);
void bytecode_emit_stmt_block(struct bytecode_emitter *emitter, struct ast_stmt_block block);
void bytecode_emit_stmt(struct bytecode_emitter *emitter, struct ast_stmt *stmt);

//
// accumulator register  = BYTECODE_REGISTER_RCX
// temporary register    = BYTECODE_REGISTER_RDX
// function return value = BYTECODE_REGISTER_RAX
//

void bytecode_data_insert_string(struct bytecode_emitter *emitter, const uint8_t *string)
{
    int size = length_of_string(string);
    for (int i = 0; i < size; ++i) {
        *emitter->data_cursor++ = string[i];
    }
    *emitter->data_cursor++ = '\0';
}

void bytecode_emit(struct bytecode_emitter *emitter, uint64_t raw_instr)
{
    if (emitter->text_cursor >= emitter->program_text + emitter->text_size) {
        uint64_t old_size = emitter->text_size;
        emitter->text_size *= 2;
        emitter->program_text = realloc(emitter->program_text, emitter->text_size);
        if (!emitter->program_text) {
            printf("fatal error: failed to reallocate text-segment!!!\n");
            exit(1);
        }
        emitter->text_cursor = emitter->program_text + old_size;
    }
    *emitter->text_cursor++ = raw_instr;
}

uint64_t *bytecode_emitter_mark_patch_source(struct bytecode_emitter *emitter)
{
    return emitter->text_cursor;
}

uint64_t bytecode_emitter_mark_patch_target(struct bytecode_emitter *emitter)
{
    return emitter->text_cursor - emitter->program_text;
}

void bytecode_emit_expression_sub(struct bytecode_emitter *emitter, struct ast_expr *left, struct ast_expr *right)
{
    bytecode_emit_expression(emitter, left);
    bytecode_emit(emitter, _push_reg(BYTECODE_REGISTER_RCX));
    bytecode_emit_expression(emitter, right);
    bytecode_emit(emitter, _pop_i64_reg(BYTECODE_REGISTER_RDX));
    bytecode_emit(emitter, _sub_reg_reg(BYTECODE_REGISTER_RCX, BYTECODE_REGISTER_RDX));
}

void bytecode_emit_expression_add(struct bytecode_emitter *emitter, struct ast_expr *left, struct ast_expr *right)
{
    bytecode_emit_expression(emitter, left);
    bytecode_emit(emitter, _push_reg(BYTECODE_REGISTER_RCX));
    bytecode_emit_expression(emitter, right);
    bytecode_emit(emitter, _pop_i64_reg(BYTECODE_REGISTER_RDX));
    bytecode_emit(emitter, _add_reg_reg(BYTECODE_REGISTER_RCX, BYTECODE_REGISTER_RDX));
}

void bytecode_emit_expression_mul(struct bytecode_emitter *emitter, struct ast_expr *left, struct ast_expr *right)
{
    bytecode_emit_expression(emitter, left);
    bytecode_emit(emitter, _push_reg(BYTECODE_REGISTER_RCX));
    bytecode_emit_expression(emitter, right);
    bytecode_emit(emitter, _pop_i64_reg(BYTECODE_REGISTER_RDX));
    bytecode_emit(emitter, _mul_reg_reg(BYTECODE_REGISTER_RCX, BYTECODE_REGISTER_RDX));
}

void bytecode_emit_expression_div(struct bytecode_emitter *emitter, struct ast_expr *left, struct ast_expr *right)
{
    bytecode_emit_expression(emitter, left);
    bytecode_emit(emitter, _push_reg(BYTECODE_REGISTER_RCX));
    bytecode_emit_expression(emitter, right);
    bytecode_emit(emitter, _pop_i64_reg(BYTECODE_REGISTER_RDX));
    bytecode_emit(emitter, _div_reg_reg(BYTECODE_REGISTER_RCX, BYTECODE_REGISTER_RDX));
}

void bytecode_emit_expression_greater_than(struct bytecode_emitter *emitter, struct ast_expr *left, struct ast_expr *right)
{
    bytecode_emit_expression(emitter, left);
    bytecode_emit(emitter, _push_reg(BYTECODE_REGISTER_RCX));
    bytecode_emit_expression(emitter, right);
    bytecode_emit(emitter, _pop_i64_reg(BYTECODE_REGISTER_RDX));

    bytecode_emit(emitter, _cmp_reg_reg(BYTECODE_REGISTER_RDX, BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, _jg_imm());

    uint64_t *true_patch = bytecode_emitter_mark_patch_source(emitter);
    bytecode_emit(emitter, -1);

    bytecode_emit(emitter, _mov_i64_reg_imm(BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, 0);
    bytecode_emit(emitter, _jmp_imm());
    uint64_t *end_patch = bytecode_emitter_mark_patch_source(emitter);
    bytecode_emit(emitter, -1);

    int true_target = bytecode_emitter_mark_patch_target(emitter);
    *true_patch = true_target;

    bytecode_emit(emitter, _mov_i64_reg_imm(BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, 1);

    int end_target = bytecode_emitter_mark_patch_target(emitter);
    *end_patch = end_target;
}

void bytecode_emit_expression_less_than(struct bytecode_emitter *emitter, struct ast_expr *left, struct ast_expr *right)
{
    bytecode_emit_expression(emitter, left);
    bytecode_emit(emitter, _push_reg(BYTECODE_REGISTER_RCX));
    bytecode_emit_expression(emitter, right);
    bytecode_emit(emitter, _pop_i64_reg(BYTECODE_REGISTER_RDX));

    bytecode_emit(emitter, _cmp_reg_reg(BYTECODE_REGISTER_RDX, BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, _jl_imm());

    uint64_t *true_patch = bytecode_emitter_mark_patch_source(emitter);
    bytecode_emit(emitter, -1);

    bytecode_emit(emitter, _mov_i64_reg_imm(BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, 0);
    bytecode_emit(emitter, _jmp_imm());
    uint64_t *end_patch = bytecode_emitter_mark_patch_source(emitter);
    bytecode_emit(emitter, -1);

    int true_target = bytecode_emitter_mark_patch_target(emitter);
    *true_patch = true_target;

    bytecode_emit(emitter, _mov_i64_reg_imm(BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, 1);

    int end_target = bytecode_emitter_mark_patch_target(emitter);
    *end_patch = end_target;
}

void bytecode_emit_expression_equal(struct bytecode_emitter *emitter, struct ast_expr *left, struct ast_expr *right)
{
    bytecode_emit_expression(emitter, left);
    bytecode_emit(emitter, _push_reg(BYTECODE_REGISTER_RCX));
    bytecode_emit_expression(emitter, right);
    bytecode_emit(emitter, _pop_i64_reg(BYTECODE_REGISTER_RDX));

    bytecode_emit(emitter, _cmp_reg_reg(BYTECODE_REGISTER_RDX, BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, _jz_imm());

    uint64_t *true_patch = bytecode_emitter_mark_patch_source(emitter);
    bytecode_emit(emitter, -1);

    bytecode_emit(emitter, _mov_i64_reg_imm(BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, 0);
    bytecode_emit(emitter, _jmp_imm());
    uint64_t *end_patch = bytecode_emitter_mark_patch_source(emitter);
    bytecode_emit(emitter, -1);

    int true_target = bytecode_emitter_mark_patch_target(emitter);
    *true_patch = true_target;

    bytecode_emit(emitter, _mov_i64_reg_imm(BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, 1);

    int end_target = bytecode_emitter_mark_patch_target(emitter);
    *end_patch = end_target;
}

void bytecode_emit_expression_and(struct bytecode_emitter *emitter, struct ast_expr *left, struct ast_expr *right)
{
    bytecode_emit_expression(emitter, left);
    bytecode_emit(emitter, _cmp_reg_imm(BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, 0);
    bytecode_emit(emitter, _jz_imm());

    uint64_t *patch = bytecode_emitter_mark_patch_source(emitter);
    bytecode_emit(emitter, -1);

    bytecode_emit_expression(emitter, right);

    int target = bytecode_emitter_mark_patch_target(emitter);
    *patch = target;
}

void bytecode_emit_expression_or(struct bytecode_emitter *emitter, struct ast_expr *left, struct ast_expr *right)
{
    bytecode_emit_expression(emitter, left);
    bytecode_emit(emitter, _cmp_reg_imm(BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, 0);
    bytecode_emit(emitter, _jnz_imm());

    uint64_t *patch = bytecode_emitter_mark_patch_source(emitter);
    bytecode_emit(emitter, -1);

    bytecode_emit_expression(emitter, right);

    int target = bytecode_emitter_mark_patch_target(emitter);
    *patch = target;
}

void bytecode_emit_expression_assign(struct bytecode_emitter *emitter, struct ast_expr *left_expr, struct ast_expr *right_expr)
{
    bytecode_emit_expression(emitter, left_expr);
    bytecode_emit(emitter, _push_reg(BYTECODE_REGISTER_R9));

    bytecode_emit_expression(emitter, right_expr);

    bytecode_emit(emitter, _pop_i64_reg(BYTECODE_REGISTER_R9));
    bytecode_emit(emitter, _memw_reg_reg(BYTECODE_REGISTER_R9, BYTECODE_REGISTER_RCX));
}

struct bytecode_data_string
{
    const uint8_t *string;
    char *storage;
};
struct bytecode_data_string *bytecode_data_strings;

int bytecode_data_find_string(struct bytecode_emitter *emitter, const uint8_t *string)
{
    for (size_t i = 0; i < buf_len(bytecode_data_strings); ++i) {
        struct bytecode_data_string data_string = bytecode_data_strings[i];
        if (data_string.string == string) {
            return data_string.storage - emitter->program_data;
        }
    }
    return -1;
}

void bytecode_emit_expression_immediate_string_(struct bytecode_emitter *emitter, const uint8_t *string)
{
    if (bytecode_data_find_string(emitter, string) == -1) {
        char *storage = emitter->data_cursor;
        bytecode_data_insert_string(emitter, string);
        buf_push(bytecode_data_strings, ((struct bytecode_data_string) {
                .string = string,
                .storage = storage
        }));
    }
}

void bytecode_emit_expression_immediate_string(struct bytecode_emitter *emitter, struct ast_expr *expr)
{
    bytecode_emit_expression_immediate_string_(emitter, expr->string_val);
}

void bytecode_emit_expression_immediate_int(struct bytecode_emitter *emitter, struct ast_expr *expr)
{
    bytecode_emit(emitter, _mov_i64_reg_imm(BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, expr->int_val);
}

void bytecode_emit_expression_identifier(struct bytecode_emitter *emitter, struct ast_expr *expr)
{
    assert(expr->symbol);
    struct symbol *symbol = expr->symbol;
    assert(symbol);

    if (symbol->decl) {
        bytecode_emit(emitter, _lea_bss_reg_imm(BYTECODE_REGISTER_R9));
    } else {
        bytecode_emit(emitter, _lea_lcl_reg_imm(BYTECODE_REGISTER_R9));
    }

    bytecode_emit(emitter, symbol->address);
    bytecode_emit(emitter, _memr_i64_reg_reg(BYTECODE_REGISTER_RCX, BYTECODE_REGISTER_R9));
}

void _bytecode_emit_expression_binary(struct bytecode_emitter *emitter, enum token_kind op, struct ast_expr *left_expr, struct ast_expr *right_expr)
{
    switch (op) {
    case '+': {
        bytecode_emit_expression_add(emitter, left_expr, right_expr);
    } break;
    case '-': {
        bytecode_emit_expression_sub(emitter, left_expr, right_expr);
    } break;
    case '*': {
        bytecode_emit_expression_mul(emitter, left_expr, right_expr);
    } break;
    case '/': {
        bytecode_emit_expression_div(emitter, left_expr, right_expr);
    } break;
    case '>': {
        bytecode_emit_expression_greater_than(emitter, left_expr, right_expr);
    } break;
    case '<': {
        bytecode_emit_expression_less_than(emitter, left_expr, right_expr);
    } break;
    case '=': {
        bytecode_emit_expression_assign(emitter, left_expr, right_expr);
    } break;
    case TOKEN_KIND_EQUAL: {
        bytecode_emit_expression_equal(emitter, left_expr, right_expr);
    } break;
    case TOKEN_KIND_AND: {
        bytecode_emit_expression_and(emitter, left_expr, right_expr);
    } break;
    case TOKEN_KIND_OR: {
        bytecode_emit_expression_or(emitter, left_expr, right_expr);
    } break;
    default: {
    } break;
    }
}

void bytecode_emit_expression_binary(struct bytecode_emitter *emitter, struct ast_expr_binary expr)
{
    _bytecode_emit_expression_binary(emitter, expr.op, expr.left_expr, expr.right_expr);
}

void bytecode_emit_expression_ternary(struct bytecode_emitter *emitter, struct ast_expr_ternary expr)
{
    bytecode_emit_expression(emitter, expr.condition);
    bytecode_emit(emitter, _cmp_reg_imm(BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, 0);
    bytecode_emit(emitter, _jz_imm());

    uint64_t *else_branch_patch = bytecode_emitter_mark_patch_source(emitter);
    bytecode_emit(emitter, -1);

    bytecode_emit_expression(emitter, expr.then_expr);
    bytecode_emit(emitter, _jmp_imm());

    uint64_t *end_branch_patch = bytecode_emitter_mark_patch_source(emitter);
    bytecode_emit(emitter, -1);

    int else_mark = bytecode_emitter_mark_patch_target(emitter);
    bytecode_emit_expression(emitter, expr.else_expr);
    int end_mark = bytecode_emitter_mark_patch_target(emitter);

    *else_branch_patch = else_mark;
    *end_branch_patch  = end_mark;
}

void bytecode_emit_expression_inc(struct bytecode_emitter *emitter, struct ast_expr *expr)
{
    bytecode_emit_expression(emitter, expr);
    bytecode_emit(emitter, _add_i64_reg_imm(BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, 1);
    bytecode_emit(emitter, _memw_reg_reg(BYTECODE_REGISTER_R9, BYTECODE_REGISTER_RCX));
}

void bytecode_emit_expression_dec(struct bytecode_emitter *emitter, struct ast_expr *expr)
{
    bytecode_emit_expression(emitter, expr);
    bytecode_emit(emitter, _sub_i64_reg_imm(BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, 1);
    bytecode_emit(emitter, _memw_reg_reg(BYTECODE_REGISTER_R9, BYTECODE_REGISTER_RCX));
}

void bytecode_emit_expression_dereference(struct bytecode_emitter *emitter, struct ast_expr *expr)
{
    // NOTE: The expression could be of type AST_EXPR_UNARY, which usually
    // means we are derefencing a multi-level pointer. We need to find the innermost expression.
    while (expr->kind == AST_EXPR_UNARY) {
        assert(expr->unary.op == '*');
        expr = expr->unary.expr;
    }

    if (expr->kind == AST_EXPR_FIELD) {
        // NOTE: support expressions of type FIELD ACCESS
        assert(expr->field.expr->symbol);
        struct symbol *symbol = expr->field.expr->symbol;
        assert(symbol);

        printf("dereference field access %s\n", symbol->name);

        int field_offset = 0;
        struct type *type = NULL;
        for (size_t i = 0; i < symbol->type->aggregate.fields_count; ++i) {
            struct type_field field = symbol->type->aggregate.fields[i];
            if (field.name == expr->field.name) {
                type = field.type;
                break;
            }
            field_offset += type_sizeof(field.type);
        }

        if (symbol->decl) {
            bytecode_emit(emitter, _lea_bss_reg_imm(BYTECODE_REGISTER_R9));
        } else {
            bytecode_emit(emitter, _lea_lcl_reg_imm(BYTECODE_REGISTER_R9));
        }

        bytecode_emit(emitter, symbol->address);

        bytecode_emit(emitter, _mov_i64_reg_imm(BYTECODE_REGISTER_R11));
        bytecode_emit(emitter, field_offset);
        bytecode_emit(emitter, _add_reg_reg(BYTECODE_REGISTER_R9, BYTECODE_REGISTER_R11));

        while (type->kind == TYPE_PTR) {
            // NOTE: if the type we are derefencing is a pointer, we need to do an additional read.
            // This is because we load the address of a local variable to read from, but the value of the variable
            // is the address of the variable we are pointing to; we must then read the value of that variable!
            printf("identifier %s is ptr, require another read\n", symbol->name);
            bytecode_emit(emitter, _memr_i64_reg_reg(BYTECODE_REGISTER_R9, BYTECODE_REGISTER_R9));
            type = type->ptr.elem;
        }

        bytecode_emit(emitter, _memr_i64_reg_reg(BYTECODE_REGISTER_RCX, BYTECODE_REGISTER_R9));
    } else if (expr->kind == AST_EXPR_UNARY) {
        printf("DEREFENCING UNARY EXPR SHOULD NOT HAPPEN\n");
        assert(0);
    } else {
        // NOTE: dereference an identifier
        assert(expr->symbol);
        struct symbol *symbol = expr->symbol;
        assert(symbol);
        struct type *type = symbol->type;
        assert(type);

        printf("dereference identifier %s\n", symbol->name);

        if (symbol->decl) {
            bytecode_emit(emitter, _lea_bss_reg_imm(BYTECODE_REGISTER_R9));
        } else {
            bytecode_emit(emitter, _lea_lcl_reg_imm(BYTECODE_REGISTER_R9));
        }

        bytecode_emit(emitter, symbol->address);

        while (type->kind == TYPE_PTR) {
            // NOTE: if the type we are derefencing is a pointer, we need to do an additional read.
            // This is because we load the address of a local variable to read from, but the value of the variable
            // is the address of the variable we are pointing to; we must then read the value of that variable!
            printf("identifier %s is ptr, require another read\n", symbol->name);
            bytecode_emit(emitter, _memr_i64_reg_reg(BYTECODE_REGISTER_R9, BYTECODE_REGISTER_R9));
            type = type->ptr.elem;
        }

        bytecode_emit(emitter, _memr_i64_reg_reg(BYTECODE_REGISTER_RCX, BYTECODE_REGISTER_R9));
    }
}

void bytecode_emit_expression_address(struct bytecode_emitter *emitter, struct ast_expr *expr)
{
    if (expr->kind == AST_EXPR_FIELD) {
        assert(expr->field.expr->symbol);
        struct symbol *symbol = expr->field.expr->symbol;
        assert(symbol);

        int field_offset = 0;
        for (size_t i = 0; i < symbol->type->aggregate.fields_count; ++i) {
            struct type_field field = symbol->type->aggregate.fields[i];
            if (field.name == expr->field.name) {
                break;
            }
            field_offset += type_sizeof(field.type);
        }

        if (symbol->decl) {
            bytecode_emit(emitter, _lea_bss_reg_imm(BYTECODE_REGISTER_RCX));
        } else {
            bytecode_emit(emitter, _lea_lcl_reg_imm(BYTECODE_REGISTER_RCX));
        }

        bytecode_emit(emitter, symbol->address);

        bytecode_emit(emitter, _mov_i64_reg_imm(BYTECODE_REGISTER_R11));
        bytecode_emit(emitter, field_offset);
        bytecode_emit(emitter, _add_reg_reg(BYTECODE_REGISTER_RCX, BYTECODE_REGISTER_R11));
    } else {
        assert(expr->symbol);
        struct symbol *symbol = expr->symbol;
        assert(symbol);

        if (symbol->decl) {
            bytecode_emit(emitter, _lea_bss_reg_imm(BYTECODE_REGISTER_RCX));
        } else {
            bytecode_emit(emitter, _lea_lcl_reg_imm(BYTECODE_REGISTER_RCX));
        }

        bytecode_emit(emitter, symbol->address);
    }
}

void _bytecode_emit_expression_unary(struct bytecode_emitter *emitter, enum token_kind op, struct ast_expr *expr)
{
    switch (op) {
    case '+': {
    } break;
    case '-': {
    } break;
    case '*': {
        bytecode_emit_expression_dereference(emitter, expr);
    } break;
    case '&': {
        bytecode_emit_expression_address(emitter, expr);
    } break;
    case '~': {
    } break;
    case TOKEN_KIND_INC: {
        bytecode_emit_expression_inc(emitter, expr);
    } break;
    case TOKEN_KIND_DEC: {
        bytecode_emit_expression_dec(emitter, expr);
    } break;
    default: {
    } break;
    }
}

void bytecode_emit_expression_unary(struct bytecode_emitter *emitter, struct ast_expr_unary expr)
{
    _bytecode_emit_expression_unary(emitter, expr.op, expr.expr);
}

void bytecode_emit_expression_call(struct bytecode_emitter *emitter, struct ast_expr_call call)
{
    struct symbol *symbol = symbol_get(emitter->resolver, call.expr->string_val);
    if (symbol->kind == SYMBOL_FUNC_FOREIGN) {
        // emit function name and corresponding library to data segment
        bytecode_emit_expression_immediate_string(emitter, call.expr);
        bytecode_emit_expression_immediate_string_(emitter, symbol->decl->foreign_func_decl.lib);

        // emit code for arguments
        for (int i = 0; i < call.args_count; ++i) {
            struct ast_expr *arg = call.args[0];
            // bytecode_emit_expression_call_argument(emitter, arg, i);

            // TODO: replace with proper recursive descent of arguments
            struct symbol *arg_symbol = symbol_get(emitter->resolver, arg->string_val);
            bytecode_emit_expression_immediate_string(emitter, arg_symbol->decl->const_decl.expr);
            int format_string = bytecode_data_find_string(emitter, arg_symbol->decl->const_decl.expr->string_val);
            bytecode_emit(emitter, _lea_bss_reg_imm(BYTECODE_REGISTER_RDI));
            bytecode_emit(emitter, format_string);

            /*
            const uint8_t *arg_name = symbol->decl->func_decl.params[i].name;
            uint64_t arg_address = symbol->decl->func_decl.params[i].address;
            printf("%s address %d\n", arg_name, arg_address);

            bytecode_emit_expression(emitter, arg);
            bytecode_emit(emitter, _lea_lcl_reg_imm(BYTECODE_REGISTER_RDI));
            bytecode_emit(emitter, arg_address);
            */
        }

        // locate function and library name in data segment - required by upcoming instructions
        int func_name = bytecode_data_find_string(emitter, call.expr->string_val);
        bytecode_emit(emitter, _lea_bss_reg_imm(BYTECODE_REGISTER_R11));
        bytecode_emit(emitter, func_name);

        int lib_name = bytecode_data_find_string(emitter, symbol->decl->foreign_func_decl.lib);
        bytecode_emit(emitter, _lea_bss_reg_imm(BYTECODE_REGISTER_R12));
        bytecode_emit(emitter, lib_name);

        // function name and library is read off the stack, func name first, followed by lib name (push in reverse order!!!)
        bytecode_emit(emitter, _push_reg(BYTECODE_REGISTER_R12));
        bytecode_emit(emitter, _push_reg(BYTECODE_REGISTER_R11));

        bytecode_emit(emitter, _call_foreign());
        bytecode_emit(emitter, call.args_count);
        bytecode_emit(emitter, BYTECODE_REGISTER_KIND_I64);
    } else {
        for (size_t i = 0; i < call.args_count; ++i) {
            enum bytecode_register reg = bytecode_internal_call_registers[i];
            bytecode_emit_expression(emitter, call.args[i]);
            bytecode_emit(emitter, _mov_reg_reg(reg, BYTECODE_REGISTER_RCX));
        }

        bytecode_emit(emitter, _call_imm());

        uint64_t *call_patch = bytecode_emitter_mark_patch_source(emitter);
        bytecode_emit(emitter, -1);

        buf_push(emitter->call_patches, call_patch);
        buf_push(emitter->call_patches_symbol, symbol);
    }
}

void bytecode_emit_expression_field(struct bytecode_emitter *emitter, struct ast_expr *expr)
{
    if (expr->field.expr->kind == AST_EXPR_UNARY) {
        // NOTE: support unary expressions of type POINTER DEREFERENCE
        assert(expr->field.expr->unary.op == '*');
        assert(expr->field.expr->unary.expr->symbol);
        struct symbol *symbol = expr->field.expr->unary.expr->symbol;
        assert(symbol);
        assert(symbol->type);
        assert(symbol->type->kind == TYPE_PTR);

        struct type *type = symbol->type;
        while (type->kind == TYPE_PTR) {
            type = type->ptr.elem;
        }

        assert(type);

        int field_offset = 0;
        for (size_t i = 0; i < type->aggregate.fields_count; ++i) {
            struct type_field field = type->aggregate.fields[i];
            if (field.name == expr->field.name) {
                break;
            }
            field_offset += type_sizeof(field.type);
        }

        // printf("UNARY %s FIELD EXPR: offset %d\n", symbol->name, field_offset);

        bytecode_emit_expression(emitter, expr->field.expr);

        bytecode_emit(emitter, _mov_i64_reg_imm(BYTECODE_REGISTER_R11));
        bytecode_emit(emitter, field_offset);
        bytecode_emit(emitter, _add_reg_reg(BYTECODE_REGISTER_R9, BYTECODE_REGISTER_R11));

        bytecode_emit(emitter, _memr_i64_reg_reg(BYTECODE_REGISTER_RCX, BYTECODE_REGISTER_R9));
    } else {
        assert(expr->field.expr->symbol);
        struct symbol *symbol = expr->field.expr->symbol;
        assert(symbol);
        assert(symbol->type);

        if (symbol->type->kind == TYPE_STRUCT) {
            int field_offset = 0;
            for (size_t i = 0; i < symbol->type->aggregate.fields_count; ++i) {
                struct type_field field = symbol->type->aggregate.fields[i];
                if (field.name == expr->field.name) {
                    break;
                }
                field_offset += type_sizeof(field.type);
            }

            // printf("%s.%s | offset: %d\n", symbol->name, expr->field.name, field_offset);

            if (symbol->decl) {
                bytecode_emit(emitter, _lea_bss_reg_imm(BYTECODE_REGISTER_R9));
            } else {
                bytecode_emit(emitter, _lea_lcl_reg_imm(BYTECODE_REGISTER_R9));
            }

            bytecode_emit(emitter, symbol->address);

            bytecode_emit(emitter, _mov_i64_reg_imm(BYTECODE_REGISTER_R11));
            bytecode_emit(emitter, field_offset);
            bytecode_emit(emitter, _add_reg_reg(BYTECODE_REGISTER_R9, BYTECODE_REGISTER_R11));

            bytecode_emit(emitter, _memr_i64_reg_reg(BYTECODE_REGISTER_RCX, BYTECODE_REGISTER_R9));
        } else {
            int field_value = -1;
            for (size_t i = 0; i < symbol->decl->enum_decl.items_count; ++i) {
                struct ast_enum_item item = symbol->decl->enum_decl.items[i];

                if (item.expr) {
                    assert(item.expr->res.is_const);
                    field_value = item.expr->res.val;
                } else {
                    ++field_value;
                }

                if (item.name == expr->field.name) {
                    break;
                }
            }

            // printf("%s.%s | value: %d\n", symbol->name, expr->field.name, field_value);

            bytecode_emit(emitter, _mov_i64_reg_imm(BYTECODE_REGISTER_RCX));
            bytecode_emit(emitter, field_value);
        }
    }
}

void bytecode_emit_expression(struct bytecode_emitter *emitter, struct ast_expr *expr)
{
    switch (expr->kind) {
    case AST_EXPR_FLOAT_LITERAL:
    case AST_EXPR_CAST:
    case AST_EXPR_INDEX:
        break;
    case AST_EXPR_FIELD:
        bytecode_emit_expression_field(emitter, expr);
        break;
    case AST_EXPR_IDENTIFIER: {
        bytecode_emit_expression_identifier(emitter, expr);
    } break;
    case AST_EXPR_INT_LITERAL: {
        bytecode_emit_expression_immediate_int(emitter, expr);
    } break;
    case AST_EXPR_STRING_LITERAL: {
        bytecode_emit_expression_immediate_string(emitter, expr);
    } break;
    case AST_EXPR_UNARY: {
        bytecode_emit_expression_unary(emitter, expr->unary);
    } break;
    case AST_EXPR_BINARY: {
        bytecode_emit_expression_binary(emitter, expr->binary);
    } break;
    case AST_EXPR_TERNARY: {
        bytecode_emit_expression_ternary(emitter, expr->ternary);
    } break;
    case AST_EXPR_CALL: {
        bytecode_emit_expression_call(emitter, expr->call);
    } break;
    default: {
    } break;
    }
}

void bytecode_emit_stmt_assign(struct bytecode_emitter *emitter, struct ast_stmt_assign stmt_assign)
{
    if (stmt_assign.left_expr && stmt_assign.right_expr) {
        _bytecode_emit_expression_binary(emitter, stmt_assign.op, stmt_assign.left_expr, stmt_assign.right_expr);
    } else {
        _bytecode_emit_expression_unary(emitter, stmt_assign.op, stmt_assign.left_expr);
    }
}

void bytecode_emit_stmt_init_local(struct bytecode_emitter *emitter, struct ast_stmt_init stmt_init)
{
    bytecode_emit_expression(emitter, stmt_init.expr);
    bytecode_emit(emitter, _mov_lcl_reg(BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, stmt_init.address);
}

void bytecode_emit_stmt_decl_local(struct bytecode_emitter *emitter, struct ast_stmt_decl stmt_decl)
{
    if (stmt_decl.expr) {
        bytecode_emit_expression(emitter, stmt_decl.expr);
    }
    bytecode_emit(emitter, _mov_lcl_reg(BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, stmt_decl.address);
}

void bytecode_emit_stmt_if(struct bytecode_emitter *emitter, struct ast_stmt_if stmt)
{
    uint64_t **end_branch_patches = NULL;

    bytecode_emit_expression(emitter, stmt.condition);
    bytecode_emit(emitter, _cmp_reg_imm(BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, 0);
    bytecode_emit(emitter, _jz_imm());
    uint64_t *elseif_branch_patch = bytecode_emitter_mark_patch_source(emitter);
    bytecode_emit(emitter, -1);

    bytecode_emit_stmt_block(emitter, stmt.then_block);
    bytecode_emit(emitter, _jmp_imm());
    uint64_t *end_branch_patch = bytecode_emitter_mark_patch_source(emitter);
    buf_push(end_branch_patches, end_branch_patch);
    bytecode_emit(emitter, -1);

    int elseif_mark = bytecode_emitter_mark_patch_target(emitter);

    for (int i = 0; i < stmt.else_ifs_count; ++i) {
        *elseif_branch_patch = elseif_mark;

        struct ast_else_if elseif = stmt.else_ifs[i];
        bytecode_emit_expression(emitter, elseif.condition);
        bytecode_emit(emitter, _cmp_reg_imm(BYTECODE_REGISTER_RCX));
        bytecode_emit(emitter, 0);
        bytecode_emit(emitter, _jz_imm());
        elseif_branch_patch = bytecode_emitter_mark_patch_source(emitter);
        bytecode_emit(emitter, -1);

        bytecode_emit_stmt_block(emitter, elseif.block);
        bytecode_emit(emitter, _jmp_imm());
        end_branch_patch = bytecode_emitter_mark_patch_source(emitter);
        buf_push(end_branch_patches, end_branch_patch);
        bytecode_emit(emitter, -1);

        elseif_mark = bytecode_emitter_mark_patch_target(emitter);
    }
    *elseif_branch_patch = elseif_mark;

    bytecode_emit_stmt_block(emitter, stmt.else_block);
    int end_mark = bytecode_emitter_mark_patch_target(emitter);

    for (int i = 0; i < buf_len(end_branch_patches); ++i) {
        *end_branch_patches[i] = end_mark;
    }
}

void bytecode_emit_stmt_for(struct bytecode_emitter *emitter, struct ast_stmt_for stmt)
{
    int break_patches_count_old = emitter->break_patches_count;
    int continue_patches_count_old = emitter->continue_patches_count;

    if (stmt.init) {
        bytecode_emit_stmt(emitter, stmt.init);
    }

    int cond_mark = bytecode_emitter_mark_patch_target(emitter);

    if (stmt.condition) {
        bytecode_emit_expression(emitter, stmt.condition);
    } else {
        bytecode_emit(emitter, _mov_i64_reg_imm(BYTECODE_REGISTER_RCX));
        bytecode_emit(emitter, 1);
    }

    bytecode_emit(emitter, _cmp_reg_imm(BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, 0);
    bytecode_emit(emitter, _jz_imm());

    uint64_t *end_patch = bytecode_emitter_mark_patch_source(emitter);
    bytecode_emit(emitter, -1);

    bytecode_emit_stmt_block(emitter, stmt.block);

    int next_mark = bytecode_emitter_mark_patch_target(emitter);
    if (stmt.next) {
        bytecode_emit_stmt(emitter, stmt.next);
    }

    bytecode_emit(emitter, _jmp_imm());
    bytecode_emit(emitter, cond_mark);

    int end_mark = bytecode_emitter_mark_patch_target(emitter);
    *end_patch = end_mark;

    for (int i = break_patches_count_old; i < emitter->break_patches_count; ++i) {
        uint64_t *break_patch = emitter->break_patches[i];
        *break_patch = end_mark;
    }
    emitter->break_patches_count = break_patches_count_old;

    for (int i = continue_patches_count_old; i < emitter->continue_patches_count; ++i) {
        uint64_t *continue_patch = emitter->continue_patches[i];
        *continue_patch = next_mark;
    }
    emitter->continue_patches_count = continue_patches_count_old;
}

void bytecode_emit_stmt_while(struct bytecode_emitter *emitter, struct ast_stmt_while stmt)
{
    int break_patches_count_old = emitter->break_patches_count;
    int continue_patches_count_old = emitter->continue_patches_count;

    int cond_mark = bytecode_emitter_mark_patch_target(emitter);
    bytecode_emit_expression(emitter, stmt.condition);

    bytecode_emit(emitter, _cmp_reg_imm(BYTECODE_REGISTER_RCX));
    bytecode_emit(emitter, 0);
    bytecode_emit(emitter, _jz_imm());

    uint64_t *end_patch = bytecode_emitter_mark_patch_source(emitter);
    bytecode_emit(emitter, -1);

    bytecode_emit_stmt_block(emitter, stmt.block);

    int next_mark = bytecode_emitter_mark_patch_target(emitter);

    bytecode_emit(emitter, _jmp_imm());
    bytecode_emit(emitter, cond_mark);

    int end_mark = bytecode_emitter_mark_patch_target(emitter);
    *end_patch = end_mark;

    for (int i = break_patches_count_old; i < emitter->break_patches_count; ++i) {
        uint64_t *break_patch = emitter->break_patches[i];
        *break_patch = end_mark;
    }
    emitter->break_patches_count = break_patches_count_old;

    for (int i = continue_patches_count_old; i < emitter->continue_patches_count; ++i) {
        uint64_t *continue_patch = emitter->continue_patches[i];
        *continue_patch = next_mark;
    }
    emitter->continue_patches_count = continue_patches_count_old;
}

void bytecode_emit_stmt_break(struct bytecode_emitter *emitter)
{
    assert(emitter->break_patches_count < 12);
    bytecode_emit(emitter, _jmp_imm());
    uint64_t *break_patch = bytecode_emitter_mark_patch_source(emitter);
    emitter->break_patches[emitter->break_patches_count++] = break_patch;
    bytecode_emit(emitter, -1);
}

void bytecode_emit_stmt_continue(struct bytecode_emitter *emitter)
{
    assert(emitter->continue_patches_count < 12);
    bytecode_emit(emitter, _jmp_imm());
    uint64_t *continue_patch = bytecode_emitter_mark_patch_source(emitter);
    emitter->continue_patches[emitter->continue_patches_count++] = continue_patch;
    bytecode_emit(emitter, -1);
}

void bytecode_emit_stmt_return(struct bytecode_emitter *emitter, struct ast_stmt_return stmt)
{
    assert(emitter->return_patches_count < 12);
    if (stmt.expr) {
        bytecode_emit_expression(emitter, stmt.expr);
        bytecode_emit(emitter, _mov_reg_reg(BYTECODE_REGISTER_RAX, BYTECODE_REGISTER_RCX));
    }
    bytecode_emit(emitter, _jmp_imm());
    uint64_t *return_patch = bytecode_emitter_mark_patch_source(emitter);
    emitter->return_patches[emitter->return_patches_count++] = return_patch;
    bytecode_emit(emitter, -1);
}

void bytecode_emit_stmt(struct bytecode_emitter *emitter, struct ast_stmt *stmt)
{
    switch (stmt->kind) {
    case AST_STMT_CONST:
        break;
    case AST_STMT_ASSIGN: {
        bytecode_emit_stmt_assign(emitter, stmt->assign);
    } break;
    case AST_STMT_EXPR: {
        bytecode_emit_expression(emitter, stmt->expr);
    } break;
    case AST_STMT_INIT: {
        bytecode_emit_stmt_init_local(emitter, stmt->init);
    } break;
    case AST_STMT_DECL: {
        bytecode_emit_stmt_decl_local(emitter, stmt->decl);
    } break;
    case AST_STMT_BLOCK: {
        bytecode_emit_stmt_block(emitter, stmt->block);
    } break;
    case AST_STMT_IF: {
        bytecode_emit_stmt_if(emitter, stmt->if_stmt);
    } break;
    case AST_STMT_FOR: {
        bytecode_emit_stmt_for(emitter, stmt->for_stmt);
    } break;
    case AST_STMT_WHILE: {
        bytecode_emit_stmt_while(emitter, stmt->while_stmt);
    } break;
    case AST_STMT_BREAK: {
        bytecode_emit_stmt_break(emitter);
    } break;
    case AST_STMT_CONTINUE: {
        bytecode_emit_stmt_continue(emitter);
    } break;
    case AST_STMT_RETURN: {
        bytecode_emit_stmt_return(emitter, stmt->return_stmt);
    } break;
    default: {
    } break;
    }
}

void bytecode_emit_stmt_block(struct bytecode_emitter *emitter, struct ast_stmt_block block)
{
    for (int i = 0; i < block.statements_count; ++i) {
        struct ast_stmt *stmt = block.statements[i];
        bytecode_emit_stmt(emitter, stmt);
    }
}

void bytecode_emit_function(struct bytecode_emitter *emitter, struct symbol *symbol)
{
    printf("emitting bytecode for function: '%s'\n", symbol->name);

    struct ast_decl_func *decl = &symbol->decl->func_decl;
    uint64_t func_address = bytecode_emitter_mark_patch_target(emitter);
    uint64_t ar_size = 0x40;

    symbol->address = func_address;
    emitter->return_patches_count = 0;

    bytecode_emit(emitter, _begin_call_frame());
    bytecode_emit(emitter, _add_i64_reg_imm(BYTECODE_REGISTER_RSP));
    bytecode_emit(emitter, ar_size);

    for (size_t i = 0; i < decl->params_count; ++i) {
        enum bytecode_register reg = bytecode_internal_call_registers[i];
        bytecode_emit(emitter, _mov_lcl_reg(reg));
        bytecode_emit(emitter, decl->params[i].address);
    }

    bytecode_emit_stmt_block(emitter, symbol->decl->func_decl.block);
    bytecode_emit(emitter, _sub_i64_reg_imm(BYTECODE_REGISTER_RSP));
    bytecode_emit(emitter, ar_size);

    int end_mark = bytecode_emitter_mark_patch_target(emitter);
    bytecode_emit(emitter, _end_call_frame());
    bytecode_emit(emitter, _ret());

    for (int i = 0; i < emitter->return_patches_count; ++i) {
        uint64_t *return_patch = emitter->return_patches[i];
        *return_patch = end_mark;
    }
    emitter->return_patches_count = 0;
}

void bytecode_emit_var(struct bytecode_emitter *emitter, struct symbol *symbol)
{
    assert(symbol->type);
    assert(symbol->decl);


    symbol->address = emitter->data_cursor - emitter->program_data;
    emitter->data_cursor += type_sizeof(symbol->type);
    printf("allocating address '%" PRIu64 "' for global: '%s' of type %s\n", symbol->address, symbol->name, symbol->type->symbol->name);

    if (symbol->decl->var_decl.expr) {
        bytecode_emit_expression(emitter, symbol->decl->var_decl.expr);
        bytecode_emit(emitter, _lea_bss_reg_imm(BYTECODE_REGISTER_R9));
        bytecode_emit(emitter, symbol->address);
        bytecode_emit(emitter, _memw_reg_reg(BYTECODE_REGISTER_R9, BYTECODE_REGISTER_RCX));
    }
}

void bytecode_emit_const(struct bytecode_emitter *emitter, struct symbol *symbol)
{
    if (symbol->decl->const_decl.expr->kind == AST_EXPR_STRING_LITERAL) {
        bytecode_emit_expression_immediate_string(emitter, symbol->decl->const_decl.expr);
    }
}

void bytecode_emit_entry_point(struct bytecode_emitter *emitter)
{
    bytecode_emit(emitter, _call_imm());
    uint64_t *call_patch = bytecode_emitter_mark_patch_source(emitter);
    bytecode_emit(emitter, -1);
    bytecode_emit(emitter, _halt());

    struct symbol *symbol = symbol_get(emitter->resolver, emitter->entry_point);
    if (symbol) {
        buf_push(emitter->call_patches, call_patch);
        buf_push(emitter->call_patches_symbol, symbol);
    }
}

void bytecode_emitter_init(struct bytecode_emitter *emitter, struct resolver *resolver)
{
    memset(emitter, 0, sizeof(struct bytecode_emitter));
    emitter->data_size = 1024;
    emitter->text_size = 1024;
    emitter->resolver = resolver;
    emitter->program_data = malloc(emitter->data_size * sizeof(char));
    emitter->program_text = malloc(emitter->text_size * sizeof(uint64_t));
    emitter->data_cursor = emitter->program_data;
    emitter->text_cursor = emitter->program_text;
    emitter->entry_point = intern_string(u8"main");

}

void bytecode_emitter_destroy(struct bytecode_emitter *emitter)
{
    free(emitter->program_data);
    free(emitter->program_text);
    memset(emitter, 0, sizeof(struct bytecode_emitter));
}

struct compiler_options
{
    uint8_t *output_file;
};

void bytecode_generate(struct resolver *resolver, struct compiler_options *options)
{
    struct bytecode_emitter emitter;
    bytecode_emitter_init(&emitter, resolver);

    //
    // ---- ALLOCATE AND INITIALIZE GLOBAL VARIABLES ----
    //

    for (int i = 0; i < buf_len(resolver->ordered_symbols); ++i) {
        struct symbol *symbol = resolver->ordered_symbols[i];

        if (symbol->kind == SYMBOL_VAR) {
            bytecode_emit_var(&emitter, symbol);
        }
    }

    //
    // ---- EMIT JUMP TO ENTRY POINT ----
    //

    bytecode_emit_entry_point(&emitter);

    //
    // ---- STORE CONSTANT STRING VALUES
    //

    for (int i = 0; i < buf_len(resolver->ordered_symbols); ++i) {
        struct symbol *symbol = resolver->ordered_symbols[i];

        if (symbol->kind == SYMBOL_CONST) {
            bytecode_emit_const(&emitter, symbol);
        }
    }


    //
    // ---- EMIT CODE FOR FUNCTIONS ----
    //

    for (int i = 0; i < buf_len(resolver->ordered_symbols); ++i) {
        struct symbol *symbol = resolver->ordered_symbols[i];

        if (symbol->kind == SYMBOL_FUNC) {
            bytecode_emit_function(&emitter, symbol);
        }
    }

    //
    // ---- PATCH FUNCTION CALLS ----
    //

    assert(buf_len(emitter.call_patches) == buf_len(emitter.call_patches_symbol));
    for (int i = 0; i < buf_len(emitter.call_patches); ++i) {
        struct symbol *symbol = emitter.call_patches_symbol[i];
        uint64_t *call_patch = emitter.call_patches[i];
        *call_patch = symbol->address;

        if (symbol->name == emitter.entry_point) {
            emitter.entry_point_patched = true;
        }
    }

    //
    // ---- GENERATE EXECUTABLE ----
    //

    struct bytecode_header program_header = {
        .magic = { 'b', 'c', 'r' },
        .abi_version = 0x1,
        .stack_size = 2048,
        .data_size = (emitter.data_cursor - emitter.program_data) * sizeof(*emitter.program_data),
        .text_size = (emitter.text_cursor - emitter.program_text) * sizeof(*emitter.program_text)
    };

    struct bytecode_executable program = {
        .header = &program_header,
        .data_segment = emitter.program_data,
        .text_segment = emitter.program_text
    };

    printf("text size: %" PRIu64 " bytes\n", program_header.text_size);
    printf("data size: %" PRIu64 " bytes\n", program_header.data_size);
    printf("generated: %" PRIu64 " instructions\n", program_header.text_size / sizeof(*emitter.program_text));

    if (emitter.entry_point_patched) {
        const char *output_file = options->output_file ? (char*)options->output_file : "./sample.bcr";
        bytecode_write_executable(output_file, &program);
    } else {
        printf("fatal error: could not locate entry point 'main'\n");
    }

    bytecode_emitter_destroy(&emitter);
}

void dummy_bytecode_sample(void)
{
    /*
     * uint64_t mem[40];
     * fib :: (n: int) -> int {
     *     if n <= 1 return n;
     *
     *     if mem[n - 1] == 0 {
     *         mem[n - 1] = fib(n - 1);
     *     }
     *
     *     if mem[n - 2] == 0 {
     *         mem[n - 2] = fib(n - 2);
     *     }
     *
     *     return mem[n - 1] + mem[n - 2];
     * }

     * main :: () {
     *     for i := 1; i <= 40; i += 1 {
     *         printf("the %2d'th fibonacci number is %2d\n", i, fib(i));
     *     }
     * }
     */

    uint64_t __main = 4;
    uint64_t __fib = 32;

    char __unused program_data[] = {
        "the %2d'th fibonacci number is %2d\n\0"
        "printf\0"
        "/usr/lib/libc.dylib\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0"
    };

    uint64_t __unused program_text[] = {
        mov_i64_reg_imm(BYTECODE_REGISTER_RAX, __main),
        call_reg(BYTECODE_REGISTER_RAX),
        halt(),
        begin_call_frame(),
        mov_i64_reg_imm(BYTECODE_REGISTER_R10, 1),
        cmp_reg_imm(BYTECODE_REGISTER_R10, 40),
        jg_imm(30),
        mov_reg_reg(BYTECODE_REGISTER_RDI, BYTECODE_REGISTER_R10),
        call_imm(__fib),
        lea_bss_reg_imm(BYTECODE_REGISTER_RDI, 0),
        mov_reg_reg(BYTECODE_REGISTER_RSI, BYTECODE_REGISTER_R10),
        mov_reg_reg(BYTECODE_REGISTER_RDX, BYTECODE_REGISTER_RAX),
        lea_bss_reg_imm(BYTECODE_REGISTER_R11, 36),
        lea_bss_reg_imm(BYTECODE_REGISTER_R12, 43),
        push_reg(BYTECODE_REGISTER_R12),
        push_reg(BYTECODE_REGISTER_R11),
        call_foreign(3, BYTECODE_REGISTER_KIND_I32),
        inc_reg(BYTECODE_REGISTER_R10),
        jmp_imm(7),
        end_call_frame(),
        ret(),
        begin_call_frame(),
        push_reg(BYTECODE_REGISTER_R8),
        push_reg(BYTECODE_REGISTER_R9),
        push_reg(BYTECODE_REGISTER_R10),
        push_reg(BYTECODE_REGISTER_R11),
        push_reg(BYTECODE_REGISTER_R12),
        push_reg(BYTECODE_REGISTER_R13),
        mov_reg_reg(BYTECODE_REGISTER_R8, BYTECODE_REGISTER_RDI),
        cmp_reg_imm(BYTECODE_REGISTER_R8, 1),
        jle_imm(80),
        mov_reg_reg(BYTECODE_REGISTER_R9, BYTECODE_REGISTER_R8),
        dec_reg(BYTECODE_REGISTER_R9),
        mov_reg_reg(BYTECODE_REGISTER_R10, BYTECODE_REGISTER_R9),
        dec_reg(BYTECODE_REGISTER_R10),
        lea_bss_reg_imm(BYTECODE_REGISTER_R11, 63),
        mov_reg_reg(BYTECODE_REGISTER_R12, BYTECODE_REGISTER_R11),
        mov_i64_reg_imm(BYTECODE_REGISTER_R13, 8),
        mul_reg_reg(BYTECODE_REGISTER_R13, BYTECODE_REGISTER_R9),
        add_reg_reg(BYTECODE_REGISTER_R11, BYTECODE_REGISTER_R13),
        mov_i64_reg_imm(BYTECODE_REGISTER_R13, 8),
        mul_reg_reg(BYTECODE_REGISTER_R13, BYTECODE_REGISTER_R10),
        add_reg_reg(BYTECODE_REGISTER_R12, BYTECODE_REGISTER_R13),
        memr_i64_reg_reg(BYTECODE_REGISTER_RAX, BYTECODE_REGISTER_R11),
        cmp_reg_imm(BYTECODE_REGISTER_RAX, 0),
        jnz_imm(68),
        mov_reg_reg(BYTECODE_REGISTER_RDI, BYTECODE_REGISTER_R9),
        call_imm(__fib),
        memw_reg_reg(BYTECODE_REGISTER_R11, BYTECODE_REGISTER_RAX),
        memr_i64_reg_reg(BYTECODE_REGISTER_RAX, BYTECODE_REGISTER_R12),
        cmp_reg_imm(BYTECODE_REGISTER_RAX, 0),
        jnz_imm(77),
        mov_reg_reg(BYTECODE_REGISTER_RDI, BYTECODE_REGISTER_R10),
        call_imm(__fib),
        memw_reg_reg(BYTECODE_REGISTER_R12, BYTECODE_REGISTER_RAX),
        memr_i64_reg_reg(BYTECODE_REGISTER_R9, BYTECODE_REGISTER_R12),
        memr_i64_reg_reg(BYTECODE_REGISTER_R8, BYTECODE_REGISTER_R11),
        add_reg_reg(BYTECODE_REGISTER_R8, BYTECODE_REGISTER_R9),
        mov_reg_reg(BYTECODE_REGISTER_RAX, BYTECODE_REGISTER_R8),
        pop_i64_reg(BYTECODE_REGISTER_R13),
        pop_i64_reg(BYTECODE_REGISTER_R12),
        pop_i64_reg(BYTECODE_REGISTER_R11),
        pop_i64_reg(BYTECODE_REGISTER_R10),
        pop_i64_reg(BYTECODE_REGISTER_R9),
        pop_i64_reg(BYTECODE_REGISTER_R8),
        end_call_frame(),
        ret()
    };
}
