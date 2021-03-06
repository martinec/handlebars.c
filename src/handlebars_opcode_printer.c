
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <string.h>

#include "handlebars_compiler.h"
#include "handlebars_memory.h"
#include "handlebars_opcodes.h"
#include "handlebars_opcode_printer.h"
#include "handlebars_private.h"
#include "handlebars_utils.h"

struct handlebars_opcode_printer * handlebars_opcode_printer_ctor(void * ctx)
{
    struct handlebars_opcode_printer * printer = handlebars_talloc_zero(ctx, struct handlebars_opcode_printer);
    if( likely(printer != NULL) ) {
        printer->output = handlebars_talloc_strdup(printer, "");
    }
    return printer;
}

void handlebars_opcode_printer_dtor(struct handlebars_opcode_printer * printer)
{
    assert(printer != NULL);

    handlebars_talloc_free(printer);
}

char * handlebars_operand_print_append(char * str, struct handlebars_operand * operand)
{
    assert(str != NULL);
    assert(operand != NULL);

    switch( operand->type ) {
        case handlebars_operand_type_null:
            str = handlebars_talloc_strdup_append(str, "[NULL]");
            break;
        case handlebars_operand_type_boolean:
            str = handlebars_talloc_asprintf_append(str, "[BOOLEAN:%d]", operand->data.boolval);
            break;
        case handlebars_operand_type_long:
            str = handlebars_talloc_asprintf_append(str, "[LONG:%ld]", operand->data.longval);
            break;
        case handlebars_operand_type_string: {
            char * tmp = handlebars_addcslashes(operand->data.stringval, "\r\n\t");
            if( likely(tmp != NULL) ) {
                str = handlebars_talloc_asprintf_append(str, "[STRING:%s]", tmp);
                handlebars_talloc_free(tmp);
            }
            break;
        }
        case handlebars_operand_type_array: {
            char ** tmp = operand->data.arrayval;
            str = handlebars_talloc_strdup_append(str, "[ARRAY:");
            for( ; *tmp; ++tmp ) {
                if( tmp != operand->data.arrayval ) {
                    str = handlebars_talloc_strdup_append(str, ",");
                }
                str = handlebars_talloc_strdup_append(str, *tmp);
            }
            str = handlebars_talloc_strdup_append(str, "]");
            //str = handlebars_talloc_asprintf_append(str, "[ARRAY:]");
            break;
        }
    }
    return str;
}

char * handlebars_opcode_print_append(char * str, struct handlebars_opcode * opcode)
{
    const char * name = handlebars_opcode_readable_type(opcode->type);
    short num = handlebars_opcode_num_operands(opcode->type);
    
    str = handlebars_talloc_strdup_append(str, name);
    
    // @todo add option to override this
    if( num >= 1 ) {
        str = handlebars_operand_print_append(str, &opcode->op1);
    } else {
        assert(opcode->op1.type == handlebars_operand_type_null);
    }
    if( num >= 2 ) {
        str = handlebars_operand_print_append(str, &opcode->op2);
    } else {
        assert(opcode->op2.type == handlebars_operand_type_null);
    }
    if( num >= 3 ) {
        str = handlebars_operand_print_append(str, &opcode->op3);
    } else {
        assert(opcode->op3.type == handlebars_operand_type_null);
    }
    
    return str;
}

char * handlebars_opcode_print(void * ctx, struct handlebars_opcode * opcode)
{
    char * str = handlebars_talloc_strdup(ctx, "");
    return handlebars_opcode_print_append(str, opcode);
}

static void handlebars_opcode_printer_array_print(struct handlebars_opcode_printer * printer)
{
    int indent = printer->indent < 8 ? printer->indent * 2 : 16;
    char indentbuf[17];
    struct handlebars_opcode ** opcodes = printer->opcodes;
    size_t count = printer->opcodes_length;
    size_t i;
    
    memset(&indentbuf, ' ', indent);
    indentbuf[indent] = 0;
    
    for( i = 0; i < count; i++, opcodes++ ) {
        printer->output = handlebars_talloc_strdup_append(printer->output, indentbuf);
        printer->output = handlebars_opcode_print_append(printer->output, *opcodes);
        printer->output = handlebars_talloc_strdup_append(printer->output, "\n");
    }
    
    printer->output = handlebars_talloc_asprintf_append(printer->output, "%s-----\n", indentbuf);
}

void handlebars_opcode_printer_print(struct handlebars_opcode_printer * printer, struct handlebars_compiler * compiler)
{
    size_t i;
    char indentbuf[17];
    int indent;
    
    // Make indent
    indent = printer->indent < 8 ? printer->indent * 2 : 16;
    memset(&indentbuf, ' ', indent);
    indentbuf[indent] = 0;
    
    // Print misc
    printer->output = handlebars_talloc_asprintf_append(printer->output, "%sDEPTHS=%lu\n", indentbuf, compiler->depths);
    
    // Print opcodes
    printer->opcodes = compiler->opcodes;
    printer->opcodes_length = compiler->opcodes_length;
    handlebars_opcode_printer_array_print(printer);
    
    // Print children
    printer->indent++;
    
    for( i = 0; i < compiler->children_length; i++ ) {
        struct handlebars_compiler * child = *(compiler->children + i);
        handlebars_opcode_printer_print(printer, child);
        
    }
    
    printer->indent--;
}
