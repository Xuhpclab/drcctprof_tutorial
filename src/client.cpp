/* 
 *  Copyright (c) 2020-2021 Xuhpclab. All rights reserved.
 *  Licensed under the MIT License.
 *  See LICENSE file for more information.
 */

#include <iterator>
#include <vector>
#include <map>

#include "dr_api.h"
#include "drcctlib.h"
#include "drcctlib_vscodeex_format.h"

using namespace DrCCTProf;

#define DRCCTLIB_PRINTF(_FORMAT, _ARGS...) \
    DRCCTLIB_PRINTF_TEMPLATE("ins_count", _FORMAT, ##_ARGS)
#define DRCCTLIB_EXIT_PROCESS(_FORMAT, _ARGS...) \
    DRCCTLIB_CLIENT_EXIT_PROCESS_TEMPLATE("ins_count", _FORMAT, ##_ARGS)

#ifdef ARM_CCTLIB
#    define OPND_CREATE_CCT_INT OPND_CREATE_INT
#else
#    define OPND_CREATE_CCT_INT OPND_CREATE_INT32
#endif

#define TOP_REACH_NUM_SHOW 10


/*
* ctxt_hndl_exec_num_array stores every context handle's executed number
*   - its size equals the number of the context handle
*   - ctxt_hndl_exec_num_array[n] equals the executed number of context handle n
*   - all items in it have been init to 0
*/
uint64_t *ctxt_hndl_exec_num_array;

using namespace std;

// dr clean call
void
InsCount(int32_t opaqueHandle)
{
    void *drcontext = dr_get_current_drcontext();
    /*
     * add code to implement:
     * 1. get the current context handle
     *    Tip: use API drcctlib_get_context_handle();
     * 2. get the executed number of the current context handle
     *    Tip: use ctxt_hndl_exec_num_array to get and store every context handle's executed number
     * 3. add 1 for the executed number and store it in the array
     */
    
}

// analysis
void
InsTransEventCb(void *drcontext, instr_instrument_msg_t *instrument_msg)
{

    instrlist_t *bb = instrument_msg->bb;
    instr_t *instr = instrument_msg->instr;
    int32_t opaqueHandle = instrument_msg->slot;

    dr_insert_clean_call(drcontext, bb, instr, (void *)InsCount, false, 1, OPND_CREATE_CCT_INT(opaqueHandle));
}

static inline void
InitGlobalBuff()
{
    ctxt_hndl_exec_num_array = (uint64_t *)dr_raw_mem_alloc(
        CONTEXT_HANDLE_MAX * sizeof(uint64_t), DR_MEMPROT_READ | DR_MEMPROT_WRITE, NULL);
    if (ctxt_hndl_exec_num_array == NULL) {
        DRCCTLIB_EXIT_PROCESS(
            "init_global_buff error: dr_raw_mem_alloc fail ctxt_hndl_exec_num_array");
    }
}

static inline void
FreeGlobalBuff()
{
    dr_raw_mem_free(ctxt_hndl_exec_num_array, CONTEXT_HANDLE_MAX * sizeof(uint64_t));
}

static void
ClientInit(int argc, const char *argv[])
{
    InitGlobalBuff();
    drcctlib_init(DRCCTLIB_FILTER_ALL_INSTR, INVALID_FILE, InsTransEventCb, false);
}

typedef struct _output_format_t {
    context_handle_t handle;
    uint64_t count;
} output_format_t;

static void
ClientExit(void)
{
    output_format_t *output_list =
        (output_format_t *)dr_global_alloc(TOP_REACH_NUM_SHOW * sizeof(output_format_t));
    for (int32_t i = 0; i < TOP_REACH_NUM_SHOW; i++) {
        output_list[i].handle = 0;
        output_list[i].count = 0;
    }
    context_handle_t max_ctxt_hndl = drcctlib_get_global_context_handle_num();
    for (context_handle_t i = 0; i < max_ctxt_hndl; i++) {
        if (ctxt_hndl_exec_num_array[i] <= 0) {
            continue;
        }
        if (ctxt_hndl_exec_num_array[i] > output_list[0].count) {
            uint64_t min_count = ctxt_hndl_exec_num_array[i];
            int32_t min_idx = 0;
            for (int32_t j = 1; j < TOP_REACH_NUM_SHOW; j++) {
                if (output_list[j].count < min_count) {
                    min_count = output_list[j].count;
                    min_idx = j;
                }
            }
            output_list[0].count = min_count;
            output_list[0].handle = output_list[min_idx].handle;
            output_list[min_idx].count = ctxt_hndl_exec_num_array[i];
            output_list[min_idx].handle = i;
        }
    }

    output_format_t temp;
    for (int32_t i = 0; i < TOP_REACH_NUM_SHOW; i++) {
        for (int32_t j = i; j < TOP_REACH_NUM_SHOW; j++) {
            if (output_list[i].count < output_list[j].count) {
                temp = output_list[i];
                output_list[i] = output_list[j];
                output_list[j] = temp;
            }
        }
    }

    Profile::profile_t *profile = new Profile::profile_t();
    profile->add_metric_type(1, "-", "ins count");
    for (context_handle_t i = 3; i < max_ctxt_hndl; i++) {
        if (ctxt_hndl_exec_num_array[i] <= 0) {
            continue;
        }
        inner_context_t *cur_ctxt = drcctlib_get_full_cct(i);
        profile->add_sample(cur_ctxt)->append_metirc((uint64_t)ctxt_hndl_exec_num_array[i]);
        drcctlib_free_full_cct(cur_ctxt);
    }
    profile->serialize_to_file("ins-count.drcctprof");
    delete profile;

    file_t log = dr_open_file("ins-count.log", DR_FILE_WRITE_OVERWRITE | DR_FILE_ALLOW_LARGE);
    for (int32_t i = 0; i < TOP_REACH_NUM_SHOW; i++) {
        if (output_list[i].handle == 0) {
            break;
        }
        dr_fprintf(log, "NO. %d PC ", i + 1);
        drcctlib_print_backtrace_first_item(log, output_list[i].handle, true, false);
        dr_fprintf(log, "=>EXECUTION TIMES\n%lld\n=>BACKTRACE\n",
                   output_list[i].count);
        drcctlib_print_backtrace(log, output_list[i].handle, true, true, -1);
        dr_fprintf(log, "\n\n\n");
    }
    dr_close_file(log);

    dr_global_free(output_list, TOP_REACH_NUM_SHOW * sizeof(output_format_t));

    FreeGlobalBuff();
    drcctlib_exit();
}

#ifdef __cplusplus
extern "C" {
#endif

DR_EXPORT void
dr_client_main(client_id_t id, int argc, const char *argv[])
{
    dr_set_client_name("ins_count", "");
    ClientInit(argc, argv);
    dr_register_exit_event(ClientExit);
}

#ifdef __cplusplus
}
#endif