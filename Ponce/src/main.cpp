//IDA
#include <idp.hpp>
#include <dbg.hpp>
#include <loader.hpp>
#include <kernwin.hpp>

//Ponce
#include "callbacks.hpp"
#include "actions.hpp"
#include "globals.hpp"
#include "trigger.hpp"
#include "context.hpp"
#include "utils.hpp"

#include "formChoser.hpp"

//Triton
#include <api.hpp>

struct cmdOptionStruct cmdOptions;

//#include <x86Specifications.hpp>
//
//using namespace triton;
//using namespace triton::arch;
//using namespace triton::arch::x86;

/*This function is called once in the ide plugin init event to set the static configuration for triton. Architecture and memory/registry callbacks.*/
void triton_init()
{
	//We need to set the architecture for Triton
	if(inf.is_32bit())
		triton::api.setArchitecture(triton::arch::ARCH_X86);
	else if (inf.is_64bit())
		triton::api.setArchitecture(triton::arch::ARCH_X86_64);
	else{
		error("The architecture does not seem to be 32 or 64 bits. Exiting...");
	}
	// Memory access callback
	triton::api.addCallback(needConcreteMemoryValue);
	// Register access callback
	triton::api.addCallback(needConcreteRegisterValue);
}

//--------------------------------------------------------------------------
void idaapi run(int)
{

	if (AskUsingForm_c(form, 
		modcb, 
		&cmdOptions.limitInstructionsBeforeAskingUser, 
		&cmdOptions.limitTime,
		&cmdOptions.limitInstructionsTracingMode,
		&cmdOptions.showDebugInfo,
		&cmdOptions.manageSymbolicIndexing,
		&cmdOptions.enableTracingAsTainted,
		&cmdOptions.onlyTainting,
		&cmdOptions.RenameFunctionNames,
		&cmdOptions.automatedTainting,
		&cmdOptions.taintArgv0,
		&cmdOptions.taintArgc,
		&cmdOptions.taintRecv,
		&cmdOptions.taintFread
		) > 0)
	//if (AskUsingForm_c(form, modcb) > 0)
	{
		msg("%lld\n%lld\n%lld\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n", 
			cmdOptions.limitInstructionsBeforeAskingUser,
			cmdOptions.limitTime,
			cmdOptions.limitInstructionsTracingMode ,
			cmdOptions.showDebugInfo ? "true" : "false",
			cmdOptions.manageSymbolicIndexing ? "true" : "false",
			cmdOptions.enableTracingAsTainted ? "true" : "false",
			cmdOptions.onlyTainting ? "true" : "false",
			cmdOptions.RenameFunctionNames ? "true" : "false",
			cmdOptions.automatedTainting ? "true" : "false",
			cmdOptions.taintArgv0 ? "true" : "false",
			cmdOptions.taintArgc ? "true" : "false",
			cmdOptions.taintRecv ? "true" : "false",
			cmdOptions.taintFread ? "true" : "false");

		/*msg("%d\n%d\n%d\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
		cmdOptions.limitInstructionsBeforeAskingUser,
		cmdOptions.limitTime,
		cmdOptions.limitInstructionsTracingMode,
		cmdOptions.showDebugInfo ? "true" : "false",
		cmdOptions.manageSymbolicIndexing ? "true" : "false",
		cmdOptions.enableTracingAsTainted ? "true" : "false",
		cmdOptions.onlyTainting ? "true" : "false",
		cmdOptions.RenameFunctionNames ? "true" : "false",
		cmdOptions.automatedTainting ? "true" : "false",
		cmdOptions.taintArgv0 ? "true" : "false",
		cmdOptions.taintArgc ? "true" : "false",
		cmdOptions.taintRecv ? "true" : "false",
		cmdOptions.taintFread ? "true" : "false");*/

		/*msg("operand: %s\n", buf);
		msg("check = %d\n", check);
		msg("dim = %a %a %a %a\n", x_op1, y_op1, w_op1, h_op1);
		msg("bgc = %x\n", bgc);*/
	}








	if (!hooked){
		//First we ask the user to take a snapshot, -1 is to cancel so we don't run the plugin
		if (ask_for_a_snapshot() != -1)
		{
			if (!hook_to_notification_point(HT_UI, ui_callback, NULL))
			{
				warning("Could not hook ui callback");
				return;
			}
			if (!hook_to_notification_point(HT_DBG, tracer_callback, NULL))
			{
				warning("Could not hook tracer callback");
				return;
			}
		
			triton_init();
			msg("Plugin running\n");
			hooked = true;
		}
	}
}

//--------------------------------------------------------------------------
int idaapi init(void)
{
	for (int i = 0;; i++)
	{
		if (action_list[i].name == NULL){
			break;
		}
		//Here we register all the actions
		if (!register_action(*action_list[i].callback))
		{
			warning("Failed to register %s actions. Exiting Ponce plugin\n",action_list[i].name);
			return PLUGIN_SKIP;
		}	
	}
	if (AUTO_RUN)
		run(0);
	return PLUGIN_KEEP;
}

//--------------------------------------------------------------------------
void idaapi term(void)
{
	// just to be safe
	unhook_from_notification_point(HT_UI, ui_callback, NULL);
	unhook_from_notification_point(HT_DBG, tracer_callback, NULL);
	hooked = false;
}

//--------------------------------------------------------------------------
//
//      PLUGIN DESCRIPTION BLOCK
//
//--------------------------------------------------------------------------
plugin_t PLUGIN =
{
	IDP_INTERFACE_VERSION,
	0,                    // plugin flags
	init,                 // initialize
	term,                 // terminate. this pointer may be NULL.
	run,                  // invoke plugin
	"Instruction tracer sample", // long comment about the plugin
	"", // multiline help about the plugin
	"Ponce", // the preferred short name of the plugin
	"Alt-Z" // the preferred hotkey to run the plugin
};