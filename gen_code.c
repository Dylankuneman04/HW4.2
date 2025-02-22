#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include "machine_types.h"
#include "spl.tab.h"
#include "utilities.h"
#include "ast.h"
#include "bof.h"
#include "instruction.h"
#include "code.h"
#include "regname.h"
#include "id_use.h"
#include "literal_table.h"
#include "gen_code.h"
#include "code_seq.h"
#include "code_utils.h"

void gen_code_initialize() 
{
    literal_table_initialize();
}

// Adapted from FLOAT
// Requires: bf if open for writing in binary
// and prior to this scope checking and type checking have been done.
// Write all the instructions in cs to bf in order
static void gen_code_output_seq(BOFFILE bf, code_seq cs)
{
    while (!code_seq_is_empty(cs)) {
	bin_instr_t inst = code_seq_first(cs)->instr;
	instruction_write_bin_instr(bf, inst);
	cs = code_seq_rest(cs);
    }
}

// Adapted from FLOAT
// Return a header appropriate for the give code
static BOFHeader gen_code_program_header(code_seq main_cs)
{
    BOFHeader ret;
    strncpy(ret.magic, "MAGIC", 5);  // for FLOAT SRM
    ret.text_start_address = 0;
    // remember, the unit of length in the BOF format is a byte!
    ret.text_length = code_seq_size(main_cs) * BYTES_PER_WORD;
    int dsa = MAX(ret.text_length, 1024) + BYTES_PER_WORD;
    ret.data_start_address = dsa;
    ret.data_length = literal_table_size() * BYTES_PER_WORD;
    int sba = dsa
	+ ret.data_start_address
	+ ret.data_length + ret.text_length + STACK_SPACE;
    ret.stack_bottom_addr = sba;
    return ret;
}

static void gen_code_output_literals(BOFFILE bf)
{
    literal_table_start_iteration();
    while(literal_table_iteration_has_next())
    {
        word_type w = literal_table_iteration_next();
        bof_write_word(bf, w);
    }
    literal_table_end_iteration();
}

static void gen_code_output_program(BOFFILE bf, code_seq main_cs) 
{
    BOFHeader header = gen_code_program_header(main_cs);

    bof_write_header(bf, header);
    gen_code_output_seq(bf, main_cs);
    gen_code_output_literals(bf);
    bof_close(bf);
}

void gen_code_program(BOFFILE bf, block_t prog)
{
    code_seq main_cs = gen_code_blocks(prog);
    
    gen_code_output_program(bf, main_cs);

}

code_seq gen_code_blocks(block_t prog)
{
    code_seq main_cs;
    main_cs = gen_code_var_decls(prog.var_decls);

    int vars_len_in_bytes = (code_seq_size(main_cs) / 2 * BYTES_PER_WORD);
    code_seq_concat(&main_cs, gen_code_const_decls());
    code_seq_concat(&main_cs, code_utils_save_registers_for_AR());
    code_seq_concat(&main_cs, gen_code_stmt(prog.stmts));
    code_seq_concat(&main_cs, code_utils_restore_registers_from_AR());
    code_seq_concat(&main_cs, code_utils_deallocate_stack_space(vars_len_in_bytes));
    code_seq_add_to_end(main_cs, code_exit());

    return main_cs;
}