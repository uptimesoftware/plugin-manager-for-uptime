<?php

//static $LOAD_PLUGIN_LOG_LEVEL = "debug";
static $LOAD_PLUGIN_LOG_LEVEL = "info";

$plugins_dir_name = 'plugin_manager';

require_once("header.php");
include_once("../../{$plugins_dir_name}/pdb_functions.inc");
include_once("../../{$plugins_dir_name}/plugin_classes.inc");
require_once("../../{$plugins_dir_name}/plugin_functions.inc");


////////////////////////////////////////////////////////
// Setup directory variables
require_once("setup_directories.inc");


////////////////////////////////////////////////////////
// Main page logic

// setup the load_plugin.* cmd
$local_load_plugin_cmd = "load_plugin.bat";
$local_php_cmd = "{$uptime_dir}/apache/php/php.exe";
if (get_os_platform() == 'posix') {	// Posix
	$local_load_plugin_cmd = "load_plugin.sh";
	$local_php_cmd = "/usr/local/uptime/apache/bin/php";
	
	// figure out if running on Linux or Solaris
	$this_platform = php_uname("s");
	if (strripos(" " . $this_platform, 'SunOS') > 0 || strripos(" " . $this_platform, 'Unix') > 0) {
		// Solaris
		$local_php_cmd = "/opt/uptime/apache/bin/php";
	}
	elseif (strripos(" " . $this_platform, 'Linux') > 0) {
		// Linux
		$local_php_cmd = "/usr/local/uptime/apache/bin/php";
	}
}
// Get the version of the up.time Plugin Manager (from load_plugin.bat/sh)
$cmd = "\"{$main_dir}{$local_load_plugin_cmd}\" -v";
chdir($main_dir);	// change to the main plugin directory to run the loader script
$plugin_manager_version = exec( $cmd, $exec_output, $rv );
chdir($this_dir);	// change back to our original directory
$cmd = "";
$exec_output = "";



// just get the info first
// http://www.w3schools.com/PHP/php_file_upload.asp
if (isset($_FILES) && count($_FILES) > 0) {
	$error_msg = "";
	$uploaded_file = basename( $_FILES['uploadedfile']['name']);
	$target_file = fix_directory_slashes($downloads_dir . $uploaded_file);

	if ($_FILES["uploadedfile"]["error"] > 0) {
		$error_msg = "Error uploading file: " . $_FILES["uploadedfile"]["error"] . "<br/>\nSee '<a href='http://php.net/manual/en/features.file-upload.errors.php'>http://php.net/manual/en/features.file-upload.errors.php</a>' for more info.";
		$error_msg = "Error uploading file. Try again.";
		header( 'Location: index.php' ) ;
	}
	else {
		if (is_uploaded_file($_FILES["uploadedfile"]["tmp_name"]) && file_exists($_FILES["uploadedfile"]["tmp_name"])) {
			// make sure the file type is valid
			if ((($_FILES["uploadedfile"]["type"] == "application/octet-stream")
			|| ($_FILES["uploadedfile"]["type"] == "application/x-zip-compressed"))
			&& ($_FILES["uploadedfile"]["size"] > 100 && $_FILES["uploadedfile"]["size"] < 100000000)) {
				copy($_FILES["uploadedfile"]["tmp_name"], $target_file);
				unlink($_FILES["uploadedfile"]["tmp_name"]);

				$zip = open_file($target_file) or die("Error: Could not open file '{$target_file}'");
				$plugin_info = new PluginFile();
				$plugin_info = get_zip_info($zip, $plugin_info, $tmp_dir, $main_dir);

				$plugin_type = $plugin_info->get_file_type();
				if ($plugin_type == PluginTypes::Single) {
					$plugin_type = "Single plugin monitor";
					// check supportability on current platform
					if (! check_plugin_supportability($plugin_info)) {
						$plugin_type .= " <font color='red'>(NOT SUPPORTED ON CURRENT OPERATING SYSTEM!)</font>";
						$error_msg = "Invalid file type: '{$uploaded_file}'.";
					}
				}
				elseif ($plugin_type == PluginTypes::PKG) {
					$plugin_type = "Package file with multiple plugins";
				}
				else {
					$plugin_type = "Invalid plugin";
					$error_msg = "Invalid plugin file.";
				}

				close_file($zip);
				
				// check if the monitor has already been added
				// load db of installed plugins
				$installed_plugins = new InstalledPlugins();
				$installed_plugins->load($main_dir);
				
				if ($installed_plugins->pluginExists($plugin_info->get_name())) {
					// Compare versions
					// updating to newer plugin
					if ($installed_plugins->checkIfNewer($plugin_info->get_name(), $plugin_info->get_version())) {
						$plugin_type .= " - UPDATE";
					}
					// same or older version
					else {
						$plugin_type = "Invalid plugin";
						$error_msg = "Plugin is already added to up.time.";
					}
				}
			}
			else {
				$error_msg = "Invalid file type or file is too large (>100MB): '{$uploaded_file}'.";
			}
		}
		else {
			$error_msg = "Invalid file received: '{$uploaded_file}'.";
		}
	}

/*
print "<pre>";
print_r($plugin_info);
print "</pre>";
*/
	
	// SMARTY: Assign variables
	$smarty->assign( 'uploaded_full_file_name', $target_file );
	$error_msg = str_replace("\n", "<br/>", $error_msg);
	$smarty->assign( 'error_msg', $error_msg );
	
	$smarty->assign( 'file_name', $_FILES["uploadedfile"]["name"] );
	$smarty->assign( 'file_type', $_FILES["uploadedfile"]["type"] );
	$smarty->assign( 'file_size_kb', round($_FILES["uploadedfile"]["size"] / 1024, 1) );
	
	if (isset($plugin_info)) {
		// readme exists?
		$readme = "false";
		if (strlen($plugin_info->get_readme()) > 0) {
			$readme = "true";
		}
		
		$smarty->assign( 'plugin_type',  $plugin_type);
		$smarty->assign( 'plugin_name', $plugin_info->get_name() );
		$smarty->assign( 'plugin_desc', $plugin_info->get_description() );
		$smarty->assign( 'plugin_version', $plugin_info->get_version() );
		$smarty->assign( 'plugin_platforms', $plugin_info->get_platforms() );
		$smarty->assign( 'plugin_readme',  $readme);
		$smarty->assign( 'plugin_agent_script', $plugin_info->get_requires_agent_script_string() );
		$smarty->assign( 'plugin_restart_core', $plugin_info->get_restart_core_string() );
		$smarty->assign( 'plugin_on_upgrade_remove_files', $plugin_info->get_on_upgrade_remove_files_string() );
		$smarty->assign( 'plugin_on_upgrade_remove_monitors', $plugin_info->get_on_upgrade_remove_monitors_string() );
		$plugin_sub_type = $plugin_info->get_sub_type();
		$plugin_is_new = false;
		$plugin_is_update = false;
		// plugin is new?
		if ($plugin_sub_type == PluginSubType::NEWPLUGIN) {
			$plugin_is_new = true;
		}
		else {
			$plugin_is_new = false;
			// plugin is an update?
			if ($plugin_sub_type == PluginSubType::OLDVERSION) {
				$plugin_is_update = false;
			}
			else {
				$plugin_is_update = true;
			}
		}

		$smarty->assign( 'plugin_is_new',  $plugin_is_new);
		$smarty->assign( 'plugin_is_update',  $plugin_is_update);
		$smarty->assign( 'plugin_id',  $installed_plugins->getPluginID($plugin_info->get_name()));
		$smarty->assign( 'plugin_xmls', $plugin_info->get_xml_files() );
		$smarty->assign( 'plugin_monitor_names', $plugin_info->get_monitor_names() );
		$smarty->assign( 'plugin_files', $plugin_info->get_files_to_copy() );
	}

	// SMARTY: Display page
	$smarty->display('package_info.tpl');
}





////////////////////////////////////////////////////////
// install the plugin
elseif (isset($_REQUEST["install"]) && isset($_REQUEST["uploaded_full_file_name"])) {
	$error_msg = "";
	$target_file = $_REQUEST["uploaded_full_file_name"];
	$timer_start_time = timer_start_time();
	
	$restart_core = "-n";	// default is no restart
	if (isset($_REQUEST["restart_core"])) {
		if ($_REQUEST["restart_core"] == 'yes') {
			$restart_core = '-r';
		}
	}

	if (file_exists($target_file) && file_exists("{$main_dir}" . $local_load_plugin_cmd)) {
		// get new plugin file info
		$zip = open_file($target_file) or die("Error: Could not open file '{$target_file}'");
		$plugin_info = new PluginFile();
		$plugin_info = get_zip_info($zip, $plugin_info, $tmp_dir, $main_dir);
		// load db of installed plugins
		$installed_plugins = new InstalledPlugins();
		$installed_plugins->load($main_dir);

/*
print "<pre>";
print_r($plugin_info);
print "</pre>";
*/

		if ( ! $installed_plugins->pluginExists($plugin_info->get_name()) || $plugin_info->get_sub_type() == PluginSubType::UPDATE) {
			$php_cmd = fix_directory_slashes($local_php_cmd);
			//$cmd = "\"{$php_cmd}\" \"{$main_dir}load_plugin.php\" -r -f \"{$target_file}\"";
			$tmp_target_file = str_replace(" ", "UTSPACE", $target_file);	// command line args for load_plugin don't like spaces, so let's replace spaces with a special string
			$cmd = "\"{$main_dir}{$local_load_plugin_cmd}\" -l {$LOAD_PLUGIN_LOG_LEVEL} {$restart_core} -f \"{$tmp_target_file}\" 2>&1";
			chdir($main_dir);	// change to the main plugin directory to run the loader script
			$last_line = exec($cmd, $exec_output, $rv);
			chdir($this_dir);	// change back to our original directory
			
			$output = "";
			foreach ($exec_output as $value) {
				$output .= $value . "\n";
			}
			
			// if command executed successfully
			if ($rv == 0) {
				// add plugin to db
				//$ok = $installed_plugins->addNewPlugin($plugin_info);		// this gets done already in load_plugin.php
				$output .= "Plugin was installed successfully.\n";
			}
			else {
				$error_msg = "Warning: Command did not complete successfully. Plugin may not have been installed properly.";
			}
			$timer_total_time = timer_end_time($timer_start_time);
			$output .= "Done in {$timer_total_time}s.\n";
		}
		else {
			// plugin already exists
			$error_msg = "Plugin is already installed.";
		}
	}
	else {
		$error_msg = "Could not find file '{$target_file}'. Try re-uploading.";
	}
	// clean file(s)/dir(s) if it's installed
	close_file($zip);
	unlink($target_file);
	// clean download directory (should we? Yep.)
	recursiveDelete($downloads_dir);


	// SMARTY: Assign variables
	$error_msg = str_replace("\n", "<br/>", $error_msg);
	$smarty->assign('error_msg', $error_msg);
	$smarty->assign('uploaded_full_file_name', $target_file);
	$smarty->assign('output', $output);

	// SMARTY: Display page
	$smarty->display('install_package.tpl');
}




////////////////////////////////////////////////////////
// show the default page
else {
	// get list of plugins that are installed
	$loaded_plugins = new InstalledPlugins();
	$loaded_plugins->load($main_dir);
	$plugins = $loaded_plugins->getAll();
/*
	print "<pre>";
	print_r($plugins);
	print "</pre>";
*/
	$smarty->assign( 'loaded_plugins', $plugins );
	$smarty->assign( 'plugin_manager_version', $plugin_manager_version );

	// SMARTY: Display page
	$smarty->display('index.tpl');
}

// clean tmp
if ($LOAD_PLUGIN_LOG_LEVEL != 'debug') {
	recursiveDelete($tmp_dir);
}
?>