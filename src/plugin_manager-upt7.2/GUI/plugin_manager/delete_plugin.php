<?php
$plugins_dir_name = 'plugin_manager';
require_once("header.php");
include_once("../../{$plugins_dir_name}/pdb_functions.inc");
include_once("../../{$plugins_dir_name}/plugin_classes.inc");
require_once("../../{$plugins_dir_name}/plugin_functions.inc");
require_once("../../{$plugins_dir_name}/uptime_functions.inc");
require_once("../../{$plugins_dir_name}/uptime_ini_functions.inc");
include_once("../../{$plugins_dir_name}/agent_password.inc");
require_once("setup_directories.inc");	// save for last

$error_msg = "";
$errors = 0;
$restart_core = false;

if (isset($_REQUEST["id"])) {
	$id = intval(trim($_REQUEST["id"]));
	// go back if a number was not entered
	if ( ! is_int($id)) {
		header("location: index.php");
	}
}
else {
	// go back to original page
	header("location: index.php");
}

	
// since all is ok, check if they clicked yes or no
if (isset($_REQUEST["no"])) {
	// go back to index
	header("location: index.php");
}
elseif (isset($_REQUEST["yes"])) {
	// clicked on Yes, so let's try to delete the plugin
	
	// restart core?
	if (isset($_REQUEST["restart_core"]) && $_REQUEST["restart_core"] == 'yes') {
		$restart_core = true;
	}

	// Delete the plugin
	// load monitor info
	$loaded_plugins = new InstalledPlugins();
	$loaded_plugins->load($main_dir);
	$plugin = $loaded_plugins->getPluginInfo($id);
	$files  = $loaded_plugins->getFiles($id);
	$xmls   = $loaded_plugins->getXmls($id);
	
	// uninstall each/any xml monitors
	chdir($uptime_dir);
	foreach ($xmls as $xml) {
		$erdcdeleter = fix_directory_slashes($uptime_dir . "/scripts/erdcdeleter");
		$cmd = "\"{$erdcdeleter}\" -n \"{$xml["monitor"]}\" 2>&1";
		$exec_output = "";
		$last_line = exec($cmd, $exec_output, $rv);
		
		// return value is always zero, regardless if successful or error, so that can't be used..
		// There are only three messages we need to look for:
		// - Deleted "*" 0
		// - * is not loaded. 0
		// - * has monitors associated with it, which you need to delete first. 0
		if ( ! contains("Deleted \"", $last_line) && ! contains("is not loaded.", $last_line) ) {
			$errors++;
			$error_msg .= "Error: {$last_line}\n";
		}
	}
	
	// delete the leftover XML file(s) from the xml directory
	foreach ($xmls as $xml) {
		$full_file_name = fix_directory_slashes($uptime_dir . '/xml/' . $xml["xml"]);
		if (file_exists($full_file_name)) {
			if ( ! unlink($full_file_name) ) {
				$errors++;
				$error_msg .= "Error: Couldn't delete file '{$full_file_name}'\n";
			}
		}
	}
	
	if ( $errors == 0 && strlen($error_msg) == 0 ) {
		// stop the core
		if ($restart_core) {
			stop_uptime_core($agent_password, $plugins_dir);
		}
		
		// get ready to update uptime.lax file if necessary
		$lax_info = new UptimeIniFile();
		$lax_info->load($uptime_dir);

		
		// remove each file
		foreach ($files as $file) {
			$full_file_name = fix_directory_slashes("{$uptime_dir}/{$file}");
			if (file_exists($full_file_name)) {
				unlink($full_file_name);
			}
			// remove the file if it's a jar from the uptime.lax file
			$lax_info->remove_jar_file($file);
		}
		
		// update the uptime.lax file, if necessary
		$lax_info->save();
		
		// remove plugin from DB
		$loaded_plugins->deletePlugin($id);
		
		// start the core
		if ($restart_core) {
			start_uptime_core($agent_password, $plugins_dir);
		}
		
		if ($errors == 0) {
			// go back to index
			header("location: index.php");
		}
	}
	else {
		// errors exist.. but oh well
		// remove plugin from DB anyways
		//$loaded_plugins->deletePlugin($id);
		//header("location: index.php");
		
		// SMARTY: Assign variables
		/*
		$smarty->assign( 'plugin_info', $plugin );
		$smarty->assign( 'files', $files );
		$error_msg = str_replace("\n", "<br />", $error_msg);
		$smarty->assign( 'error_msg', $error_msg );

		// SMARTY: Display page
		$smarty->display('delete_plugin.tpl');
		exit(0);
		*/
	}
}

// get plugin from the ID received
$loaded_plugins = new InstalledPlugins();
$loaded_plugins->load($main_dir);
$plugin = $loaded_plugins->getPluginInfo($id);
$files  = $loaded_plugins->getFiles($id);
$xmls   = $loaded_plugins->getXmls($id);

if (is_null($plugin)) {
	// go back to original page
	//header("location: index.php");
	$error_msg .= "Error: Could not load plugins DB.";
	$errors++;
}

// SMARTY: Assign variables
$smarty->assign( 'plugin_info', $plugin );
$smarty->assign( 'files', $files );
$error_msg = str_replace("\n", "<br />", $error_msg);
$smarty->assign( 'error_msg', $error_msg );
$smarty->assign( 'errors', $errors );

// SMARTY: Display page
$smarty->display('delete_plugin.tpl');

?>