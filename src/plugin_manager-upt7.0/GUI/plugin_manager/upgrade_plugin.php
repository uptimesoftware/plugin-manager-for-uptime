<?php
$plugins_dir_name = 'plugin_manager';
require_once("header.php");
include_once("../../{$plugins_dir_name}/pdb_functions.inc");
include_once("../../{$plugins_dir_name}/plugin_classes.inc");
require_once("../../{$plugins_dir_name}/plugin_functions.inc");
require_once("../../{$plugins_dir_name}/uptime_functions.inc");
require_once("../../{$plugins_dir_name}/uptime_lax_functions.inc");
include_once("../../{$plugins_dir_name}/agent_password.inc");
require_once("setup_directories.inc");	// save for last


if (isset($_REQUEST["upgrade_step"]) && isset($_REQUEST["uploaded_full_file_name"]) && isset($_REQUEST["id"])) {
	$id = intval(trim($_REQUEST["id"]));
	$target_file = trim($_REQUEST["uploaded_full_file_name"]);
	$output = "";
	$timer_start_time = timer_start_time();
	// go back if a number was not entered
	if ( ! is_int($id)) {
		header("location: index.php");
	}
	
	// since all is ok, go ahead and uninstall (if required)
	else {
		$errors = 0;
		$error_msg = "";
		$plugin_on_upgrade_remove_files = true;
		$plugin_on_upgrade_remove_monitors = false;
		if (isset($_REQUEST['plugin_on_upgrade_remove_files'])) {
			if ($_REQUEST['plugin_on_upgrade_remove_files'] == "true") {
				$plugin_on_upgrade_remove_files = true;
			}
			else {
				$plugin_on_upgrade_remove_files = false;
			}
		}
		if (isset($_REQUEST['plugin_on_upgrade_remove_monitors'])) {
			if ($_REQUEST['plugin_on_upgrade_remove_monitors'] == "true") {
				$plugin_on_upgrade_remove_monitors = true;
			}
			else {
				$plugin_on_upgrade_remove_monitors = false;
			}
		}
		
		// restart core?
		$restart_core = false;
		if (isset($_REQUEST["restart_core"])) {
			if ($_REQUEST["restart_core"] == 'yes' && $plugin_on_upgrade_remove_files) {
				$restart_core = true;
			}
		}

		// Delete the plugin
		// load monitor info
		$loaded_plugins = new InstalledPlugins();
		$loaded_plugins->load($main_dir);
		$plugin = $loaded_plugins->getPluginInfo($id);
		$files  = $loaded_plugins->getFiles($id);
		$xmls   = $loaded_plugins->getXmls($id);
		// uninstall each/any xml monitors
		if ($plugin_on_upgrade_remove_monitors) {
			$output .= "Removing service monitors...\n";
			chdir($uptime_dir);
			foreach ($xmls as $xml) {
				$erdcdeleter = fix_directory_slashes($uptime_dir . "/scripts/erdcdeleter");
				$cmd = "\"{$erdcdeleter}\" -n \"{$xml["monitor"]}\"";
				$output .= " Removing monitor '{$xml["monitor"]}'... ";
				$last_line = exec($cmd, $exec_output, $rv);
				if ($rv != 0) {
					$errors++;
					$output .= "Error: " . implode($exec_output) . "\n";
				}
				else {
					$output .= "Success.\n";
				}
			}
			// delete the leftover XML file(s) from the xml directory
			foreach ($xmls as $xml) {
				$full_file_name = fix_directory_slashes($uptime_dir . '/xml/' . $xml["xml"]);
				if (file_exists($full_file_name)) {
					$output .= " Deleting file '{$xml["xml"]}'... ";
					$ok = unlink($full_file_name);
					if ($ok) {
						$output .= "Success.\n";
					}
					else {
						// something failed
						$errors++;
						$output .= "Failed. Check permissions on file '{$full_file_name}'.\n";
					}
				}
				else {
					$output .= " The file '{$full_file_name}' but it seems to be deleted already. Skipping.\n";
				}
			}
			$output .= "Done removing service monitors.\n";
			chdir($this_dir);
		}
		
		if ($errors == 0) {
			// get ready to update uptime.lax file if necessary
			$lax_info = new UptimeLaxFile();
			$lax_info->load($uptime_dir);
			
			// stop the core
			if ($restart_core) {
				$output .= "Stopping the up.time Data Collector (core).\n";
				stop_uptime_core($agent_password, $plugins_dir);
			}
			
			// remove each file
			if ($plugin_on_upgrade_remove_files) {
				$output .= "Removing files...\n";
				foreach ($files as $file) {
					if (file_exists($uptime_dir . $file)) {
						$output .= " Deleting file '{$file}'... ";
						$ok = unlink($uptime_dir . $file);
						if ($ok) {
							// remove the file if it's a jar from the uptime.lax file
							$lax_info->remove_jar_file($file);
							
							$output .= "Success.\n";
						}
						else {
							// something failed
							$errors++;
							$output .= "Failed. Check permissions.\n";
						}
					}
					else {
						$output .= " The file '{$file}' but it seems to be deleted already. Skipping.\n";
					}
				}
				$output .= "Done removing files.\n";
			}

			// update the uptime.lax file, if necessary
			$lax_info->save();
			
			// remove plugin from DB
			$output .= "Deleting old data from plugin database... ";
			$ok = $loaded_plugins->deletePlugin($id, $plugin_on_upgrade_remove_files, $plugin_on_upgrade_remove_monitors, false);
			if ($ok) {
				$output .= "Success.\n";
			}
			else {
				// something failed
				$errors++;
				$output .= "Failed. Could not successfully delete monitor from plugin database file(s).\n";
			}
			
			// start the core
			if ($restart_core) {
				$output .= "Starting the up.time Data Collector (core).\n";
				start_uptime_core($agent_password, $plugins_dir);
			}
			$timer_total_time = timer_end_time($timer_start_time);
			$output .= "Done in {$timer_total_time}s.\n";
		}
		else {
			// go back to index
			$error_msg .= "Found {$errors} errors!\n";
		}
	}
}
else {
	// go back to original page
	header("location: index.php");
}

// get plugin from the ID received
$loaded_plugins = new InstalledPlugins();
$loaded_plugins->load($main_dir);
$plugin = $loaded_plugins->getPluginInfo($id);
$files  = $loaded_plugins->getFiles($id);
$xmls   = $loaded_plugins->getXmls($id);

if (is_null($plugin)) {
	// go back to original page
	header("location: index.php");
}

// SMARTY: Assign variables
$smarty->assign( 'plugin_info', $plugin );
$smarty->assign( 'files', $files );
$error_msg = str_replace("\n", "<br/>", $error_msg);
$smarty->assign( 'error_msg', $error_msg);
$smarty->assign( 'uploaded_full_file_name', $target_file);
$smarty->assign( 'restart_core', $restart_core );
$smarty->assign( 'on_upgrade_remove_files', $plugin_on_upgrade_remove_files);
$smarty->assign( 'on_upgrade_remove_monitors', $plugin_on_upgrade_remove_monitors);
$smarty->assign( 'output', $output);

// SMARTY: Display page
$smarty->display('upgrade_plugin.tpl');

?>