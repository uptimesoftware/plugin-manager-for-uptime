<?php

// ####################################################################
// #                                                                  #
// # up.time Plugin Manager                                           #
// # load_plugin.php                                                  #
// #                                                                  #
// # Author: Joel Pereira @ 2011-2012                                 #
// #                                                                  #
// # Description:                                                     #
// # Main installing engine for managing up.time plugins and modules. #
// #                                                                  #
// ####################################################################
// Version of the up.time Plugin Manager
$plugin_manager_version = "1.2.0";



// import necessary file(s)
include_once('plugin_classes.inc');
include_once('plugin_functions.inc');
include_once('uptime_functions.inc');
include_once('uptime_ini_functions.inc');
include_once('agent_password.inc');
include_once('UptimePhpIniFile.inc');

////////////////////////////////////////////////////
// Variables
$zip_file = "";
$loglevel = LogLevel::INFO;			// Default log level - DEBUG, INFO, ERROR, FATAL, SILENT
$just_show_package_info = false;	// show just package info and exit (don't install)
$auto_restart_core = false;
$auto_restart_httpd = false;
$is_scripted = false;				// don't request user input if this is true
$agent_password = $agent_password;	// retrieved from agent_password.inc file

///////////////////////////////////////////////////////////////
// Parse arguments
// Command-Line Options:
// s - silent
// i - list info from package file (no installation)
// r - auto-restart core if necessary
// n - do NOT restart core if asked
// w - auto-restart httpd if necessary
// m - do NOT restart httpd if asked
// f <name> - load/install plugin/pkg
// l <log_level> - set log level (debug/info/error/fatal)
// p <local_agent_password> - used to restart the agent
$options = getopt("sirnwmf:l:p:v");
foreach ($options as $key => $value) {
	if (strtolower($key) == strtolower('s')) {
		$loglevel = LogLevel::SILENT;
	}
	elseif (strtolower($key) == strtolower('i')) {
		$just_show_package_info = true;
		//$loglevel = LogLevel::INFO;
	}
	elseif (strtolower($key) == strtolower('r')) {
		$auto_restart_core = true;
		$is_scripted = true;
	}
	elseif (strtolower($key) == strtolower('n')) {
		$auto_restart_core = false;
		$is_scripted = true;
	}
	elseif (strtolower($key) == strtolower('w')) {
		$auto_restart_httpd = true;
		$is_scripted = true;
	}
	elseif (strtolower($key) == strtolower('m')) {
		$auto_restart_httpd = false;
		$is_scripted = true;
	}
	elseif (strtolower($key) == strtolower('f')) {
		$just_show_package_info = false;
		if (strlen(trim($value)) > 0) {
			//$zip_file = $value;
			$zip_file = str_replace("UTSPACE", " ", $value);	// command line args for load_plugin don't like spaces, so let's replace spaces with a special string
		}
	}
	elseif (strtolower($key) == strtolower('l')) {
		// only accept valid values (DEBUG/INFO/ERROR/FATAL/SILENT)
		if (strtolower($value) == strtolower('debug')) {
			$loglevel = LogLevel::DEBUG;
		}
		elseif (strtolower($value) == strtolower('info')) {
			$loglevel = LogLevel::INFO;
		}
		elseif (strtolower($value) == strtolower('error')) {
			$loglevel = LogLevel::ERROR;
		}
		elseif (strtolower($value) == strtolower('fatal')) {
			$loglevel = LogLevel::FATAL;
		}
		elseif (strtolower($value) == strtolower('silent')) {
			$loglevel = LogLevel::SILENT;
		}
		else {
			print "Error: Invalid logging option '{$value}'\n";
		}
	}
	elseif (strtolower($key) == strtolower('p')) {
		if (strlen(trim($value)) > 0) {
			$agent_password = $value;
		}
	}
	elseif (strtolower($key) == strtolower('v')) {
		// print version and quit
		print "{$plugin_manager_version}\n";
		exit(0);
	}
	else {
		print "Invalid option(s): '{$key}'\n";
	}
}


// verify if the proper arguments were given
if (strlen(trim($zip_file)) == 0) {
	print "Error: No plugin/zip file given.\n";
	print "\nValid options are:\n";
	print "-s - Silent\n";
	print "-i - list info from package file (no installation)\n";
	print "-r - auto-restart CORE if necessary\n";
	print "-n - do NOT restart CORE if asked\n";
	print "-w - auto-restart HTTPD if necessary\n";
	print "-m - do NOT restart HTTPD if asked\n";
	print "-f <name> - load/install plugin/pkg\n";
	print "-l <log_level> - set log level (debug/info/error/fatal)\n";
	print "-v - up.time Plugin Manager version number\n";
	exit(1);
}


////////////////////////////////////////////////////
// Functions
function debug_print($msg) {
	global $loglevel;
	if ($loglevel == LogLevel::DEBUG) {
		print $msg . "\n";
	}
}
function print_msg($msglevel, $msg) {
	global $loglevel;
	//if (! isset($msglevel)) { $msglevel = 1; }
	if ($msglevel >= $loglevel) {
		print $msg . "\n";
	}
}
function extract_file($zip, $file_to_extract, $dir_to_extract) {
	$ok = false;
	if (get_os_platform() == 'win') {	// Windows
		$ok = $zip->extractTo($dir_to_extract, $file_to_extract);
		if (! $ok) {
			print_msg(LogLevel::ERROR, "ERROR: Could not extract file: '{$file_to_extrace}'");
		}
	}
	else {	// Posix
		// just check if it's already been extracted
		if (file_exists("{$dir_to_extract}{$file_to_extract}")) {
			$ok = true;
		}
	}
	return $ok;
}


////////////////////////////////////////////////////
// Directory Variables
// current dir
$this_dir = fix_directory_slashes(getcwd() . "/");	// current directory; should be something like "(uptime_dir)/GUI/plugin_manager"
// uptime dir
chdir("..");
$uptime_dir = fix_directory_slashes(getcwd() . "/");	// uptime base directory
// temp dir (for settings file)
$tmp_dir = fix_directory_slashes($this_dir . "temp/tmp_" . getmypid() . "/");
$downloads_dir = fix_directory_slashes($this_dir . "downloads/");
// backup files directory
$backup_dir = fix_directory_slashes($this_dir . "db/backup/");
// back to original dir
chdir($this_dir);
$plugins_dir = getcwd();

// if necessary directories don't exist, create them
if (! file_exists($tmp_dir)) {
	mkdir($tmp_dir, 0777, true) or die("ERROR: Could not create temp directory: '{$tmp_dir}'\n");
}
if (! file_exists($downloads_dir)) {
	mkdir($downloads_dir, 0777, true) or die("ERROR: Could not create downloads directory: '{$downloads_dir}'\n");
}
if (! file_exists($backup_dir)) {
	mkdir($backup_dir, 0777, true) or die("ERROR: Could not create backup directory: '{$backup_dir}'\n");
}




////////////////////////////////////////////////////
// Main code
$zip = open_file($zip_file) or die("ERROR: Could not open file '{$zip_file}'");
if ($zip == null) {
	die("Could not open file '{$zip_file}'");
}
$plugin_info = new PluginFile();
$plugin_info = get_zip_info($zip, $plugin_info, $tmp_dir, $this_dir);

// compare current plugin with installed plugins to see if it's already added (for upgrading)
$installed_plugins = new InstalledPlugins();
$installed_plugins->load($this_dir);
$plugin_is_update = false;
// check if the plugin should be upgraded (instead of a fresh install)
if ($installed_plugins->pluginExists($plugin_info->get_name()) && $installed_plugins->checkIfNewer($plugin_info->get_name(), $plugin_info->get_version())) {
	$plugin_is_update = true;
}

// debug info
if ($loglevel == LogLevel::DEBUG) {
	debug_print("################################## DEBUG INFO ##################################");
	debug_print("Log Level: {$loglevel}");
	debug_print("Restart Core? {$auto_restart_core}");
	debug_print("Restart HTTPD? {$auto_restart_httpd}");
	debug_print("Just show info? {$just_show_package_info}");
	debug_print("Zip Filename: {$zip_file}");
	debug_print("Plugin-Importer Dir: {$this_dir}");
	debug_print("Uptime Dir: {$uptime_dir}");
	debug_print("Temp Dir: {$tmp_dir}");
	debug_print("Downloads Dir: {$downloads_dir}");
	debug_print("OS Platform Detected: " . get_os_platform());
	debug_print("---------------------------------------");
	debug_print("PHP Array Object: plugin_info");
	var_dump($plugin_info);
	debug_print("---------------------------------------");
	debug_print("################################## DEBUG INFO ##################################");
}

if ($plugin_info->get_file_type() != PluginTypes::Invalid) {
	if ($just_show_package_info) {
		// just show package info (if it's not already done by DEBUG)
		print_msg(LogLevel::INFO, "Name: " . $plugin_info->get_name());
		print_msg(LogLevel::INFO, "Description: " . $plugin_info->get_description());
		print_msg(LogLevel::INFO, "Version: " . $plugin_info->get_version());
		print_msg(LogLevel::INFO, "Platforms: " . $plugin_info->get_platforms());
		print_msg(LogLevel::INFO, "Restart up.time Data Collector (Core): " . $plugin_info->get_restart_core_string());
		print_msg(LogLevel::INFO, "Requires agent-side script(s): " . $plugin_info->get_requires_agent_script_string());
		
		print_msg(LogLevel::INFO, "Number of monitors: " . count($plugin_info->get_xml_files()));
		print_msg(LogLevel::INFO, "Number of files/scripts: " . count($plugin_info->get_files_to_copy()));
		
		print_msg(LogLevel::INFO, "Is supported on this platform: " . (check_plugin_supportability($plugin_info) ? "Yes" : "NO"));
	}
	elseif (check_plugin_supportability($plugin_info)) {
		if (check_if_plugin_exists($plugin_info, $plugins_dir)) {
			$tmp_dir = fix_directory_slashes($tmp_dir);

			
			// Extract XML file(s) into zip directory
			print_msg(LogLevel::INFO, "Getting list of XML files to extract...");
			$count = 0;
			$zip_xmls = $plugin_info->get_xml_files();	// xml files in zip
			for ($i = 0; $i < count($zip_xmls); $i++) {
				$cur_xml = $zip_xmls[$i];
				$ok = extract_file($zip, $cur_xml, $tmp_dir);	// extract xml to temp dir
				$uptime_xml_dir = fix_directory_slashes($uptime_dir . "/xml/");
				if ($ok) {
					// move the xml to the xml directory
					if (rename($tmp_dir . $cur_xml, $uptime_xml_dir . remove_first_dir($cur_xml))) {
						print_msg(LogLevel::INFO, "Extracted XML file: '" . $cur_xml . "' to '" . $uptime_xml_dir . "'");
					}
					else {
						print_msg(LogLevel::ERROR, "ERROR: Could not move XML file: '" . $cur_xml . "' to '" . $uptime_xml_dir . "'");
					}
					// load XMLs
					$cmd = fix_directory_slashes("\"" . $uptime_dir . "/scripts/erdcloader\" -x " . "\"" . $uptime_xml_dir . remove_first_dir($cur_xml) . "\"");
					$last_line = exec($cmd, $exec_output, $cmd_ok);
					
					// get monitor name for DB
					$add_to_db = false;
					$monitor_name = get_monitor_name_from_xml($uptime_xml_dir . remove_first_dir($cur_xml));
					
					if ($cmd_ok == 0) {
						$add_to_db = true;
						print_msg(LogLevel::INFO, "Imported monitor definition '{$monitor_name}'");
					}
					else {
						// check for the special case if the monitor was already loaded, don't give an error
						// if contains "name is not unique", it's already loaded, so skip the XML loading
						if ( strpos($last_line, "name is not unique") > 0 ) {
							$add_to_db = true;
							print_msg(LogLevel::INFO, "Monitor definition is already loaded '{$monitor_name}'");
						}
						else {
							$add_to_db = false;
							print_msg(LogLevel::ERROR, "ERROR: Error loading monitor '{$cur_xml}': {$last_line}");
							print_r($exec_output);
						}
					}
					
					// add to plugins DB
					if ($add_to_db) {
						$count++;
						$plugin_info->add_to_monitor_names($monitor_name);
					}
				}
				else {
					print_msg(LogLevel::ERROR, "ERROR: Could not extract XML file: '" . remove_first_dir($cur_xml) . "' to '{$uptime_xml_dir}'");
				}
			}
			
			// check how many XMLs were loaded, and if not all of them were, error out here
			if (count($zip_xmls) != $count) {
				print_msg(LogLevel::ERROR, "ERROR: Error importing monitors. Imported {$count} out of " . count($zip_xmls) . " monitor(s).");
				exit(2);
			}
			else {
				print_msg(LogLevel::INFO, "Imported {$count} XML file(s).");
			}


			// stop the core
			$user_input_restart = "";
			if ($plugin_info->get_restart_core() || $plugin_info->get_restart_httpd()) {
				// monitor needs to be restarted, so let's either do it or ask if we can
				if ($auto_restart_core) {
					// stop automatically
					stop_uptime_core($agent_password, $this_dir);
					print_msg(LogLevel::INFO, "Stopped the up.time Data Collector (Core) successfully.");
				}
				elseif ($loglevel != LogLevel::SILENT && $is_scripted == false) {
					// ask to restart
					print "up.time Data Collector (Core) needs to be restarted.\nType \"yes\" to restart the core: ";
					$user_input_restart = substr(strtolower(trim(fgets(STDIN))), 0, 3);
					if ($user_input_restart == 'yes') {
						print_msg(LogLevel::INFO, "Stopping the up.time Data Collector (Core)...");
						if (stop_uptime_core($agent_password, $this_dir)) {
							print_msg(LogLevel::INFO, "Stopped the up.time Data Collector (Core) successfully.");
						}
						else {
							print_msg(LogLevel::ERROR, "ERROR: There was a problem stopping the up.time Data Collector (Core).");
						}
					}
					else {
						print_msg(LogLevel::INFO, "Not stopping.");
					}
				}
			}
			if ($plugin_info->get_restart_httpd()) {
				$user_input_restart = "";
				if ($auto_restart_httpd) {
					// stop automatically; actually we can't, since we're running this from the UI on Apache
					//stop_uptime_httpd($agent_password, $this_dir);
					//print_msg(LogLevel::INFO, "Stopped the up.time Web Server (httpd) successfully.");
				}
				elseif ($loglevel != LogLevel::SILENT && $is_scripted == false) {
					// ask to restart
					print "up.time Web Server (httpd) needs to be restarted.\nType \"yes\" to restart the web server: ";
					$user_input_restart = substr(strtolower(trim(fgets(STDIN))), 0, 3);
					if ($user_input_restart == 'yes') {
						print_msg(LogLevel::INFO, "Stopping the up.time Web Server (httpd)...");
						if (stop_uptime_httpd($agent_password, $this_dir)) {
							print_msg(LogLevel::INFO, "Stopped the up.time Web Server (httpd) successfully.");
						}
						else {
							print_msg(LogLevel::ERROR, "ERROR: There was a problem stopping the up.time Web Server (httpd).");
						}
					}
					else {
						print_msg(LogLevel::INFO, "Not stopping.");
					}
				}
			}


			// Extract files //
			
			// get ready to update uptime.lax file if necessary
			$lax_info = new UptimeIniFile();
			$lax_info->load($uptime_dir);

			print_msg(LogLevel::INFO, "Getting list of files to extract...");
			$count = 0;
			$zip_files = $plugin_info->get_files_to_copy();	// files in zip
			for ($i = 0; $i < count($zip_files); $i++) {
				$final_file_location = fix_directory_slashes($uptime_dir . remove_first_dir($zip_files[$i]));
				$successful = extract_file($zip, $zip_files[$i], $tmp_dir);	// it will use the files/ in the zip path
				if ($successful) {
					print_msg(LogLevel::DEBUG, "Extracted file: '" . $zip_files[$i] . "' to '{$tmp_dir}'");
					$count++;
					// move file; create directory(s) if required first
					$dir_name = remove_file_name_from_dir($final_file_location);
					if (! file_exists($dir_name)) {
						print_msg(LogLevel::INFO, "Creating directory for file: '{$dir_name}'");
						mkdir($dir_name, 0777, true);
					}
					
					// find out if we have to remove the old files (default is yes)
					if ( $plugin_info->get_on_upgrade_remove_files() ) {
						// remove files if they exist
						if ( file_exists($final_file_location) ) {
							// found old file, so let's delete it first
							print_msg(LogLevel::INFO, "Existing file will be replaced: '{$final_file_location}'");
							// make backup copy of the original just in case
							//copy($final_file_location, "{$final_file_location}.old");
							$ok = unlink($final_file_location);
							if ( ! $ok ) {
								print_msg(LogLevel::ERROR, "ERROR: Could not delete the old file: '{$final_file_location}'");
							}
						}
					}
					else {
						// skip, but log
						print_msg(LogLevel::INFO, "");
					}
					
					$ok = rename(fix_directory_slashes($tmp_dir . "/" . $zip_files[$i]), $final_file_location);
					if ($ok) {
						print_msg(LogLevel::INFO, "Moved file to '{$final_file_location}'");

						// set file execute permissions for POSIX systems
						if (get_os_platform() == 'posix') {
							// only set permission for specific file extension(s)
							$extension = get_file_extension($final_file_location);
							if (
								$extension == 'sh' ||
								$extension == 'php' ||
								$extension == 'pl' ||
								$extension == 'py' ||
								$extension == 'ksh' ||
								$extension == 'bash' ||
								$extension == 'perl' ||
								$extension == 'bin'
							) {
								// set execute permissions
								if ( chmod($final_file_location, 0755) ) {
									print_msg(LogLevel::INFO, "Set execute permissions (chmod 755) on '{$final_file_location}'");
								}
								else {
									print_msg(LogLevel::ERROR, "ERROR: Could not set execute permissions (chmod 755) on '{$final_file_location}'");
								}
							}
						}
						
						// add file to uptime.lax if it's a jar
						// the function will figure out if it needs to be added or not
						$lax_info->add_new_jar_file(remove_first_dir($zip_files[$i]));
					}
					else {
						print_msg(LogLevel::ERROR, "ERROR: Could not move file to '{$final_file_location}'");
					}
				}
				else {
					print_msg(LogLevel::ERROR, "ERROR: Could not extract file: '" . $zip_files[$i] . "' to '{$tmp_dir}'");
				}
			}
			print_msg(LogLevel::INFO, "Extracted/moved {$count} file(s).");
			// clean up "files" directory
			if ($loglevel == LogLevel::DEBUG) {
				print_msg(LogLevel::DEBUG, "Temp folder '{$tmp_dir}' will not be cleaned up in debug mode.");
			}
			else {
				recursiveDelete($tmp_dir);
			}

			// update uptime.lax file, if necessary
			$lax_info->save();
			
			
			
			
			// get ready to update PHP.INI file if necessary
			$php_ini = new UptimePhpIniFile();
			$php_ini->updatePhpIni($uptime_dir, $plugin_info);
			
			
			
			
			
			// start the core
			if ($plugin_info->get_restart_core()) {
				// monitor needs to be restarted, so let's either do it or ask if we can
				if ($auto_restart_core) {
					// start automatically; actually we can't, so just display a message
					start_uptime_core($agent_password, $this_dir);
					print_msg(LogLevel::INFO, "Started the up.time Data Collector (Core) successfully.");
				}
				elseif ($loglevel != LogLevel::SILENT) {
					if ($user_input_restart == 'yes') {
						print_msg(LogLevel::INFO, "Starting the up.time Data Collector (Core)...");
						if (restart_uptime_core($agent_password, $this_dir)) {
							print_msg(LogLevel::INFO, "Started the up.time Data Collector (Core) successfully.");
						}
						else {
							print_msg(LogLevel::ERROR, "ERROR: There was a problem starting the up.time Data Collector (Core).");
						}
					}
					elseif ($is_scripted) {
						print_msg(LogLevel::INFO, "-----------------------------------------------------------------------------------------------------");
						print_msg(LogLevel::INFO, "WARNING: Plugin may not work properly until the up.time Data Collector service (core) is restarted!");
						print_msg(LogLevel::INFO, "-----------------------------------------------------------------------------------------------------");
					}
					else {
						print_msg(LogLevel::INFO, "Not starting.");
					}
				}
			}
			if ($plugin_info->get_restart_httpd()) {
				// monitor needs to be restarted, so let's either do it or ask if we can
				if ($auto_restart_httpd) {
					// start automatically
					start_uptime_httpd($agent_password, $this_dir);
					print_msg(LogLevel::INFO, "Started the up.time Web Server (httpd) successfully.");
				}
				elseif ($loglevel != LogLevel::SILENT) {
					if ($user_input_restart == 'yes') {
						print_msg(LogLevel::INFO, "Starting the up.time Web Server (httpd)...");
						if (restart_uptime_httpd($agent_password, $this_dir)) {
							print_msg(LogLevel::INFO, "Started the up.time Web Server (httpd) successfully.");
						}
						else {
							print_msg(LogLevel::ERROR, "ERROR: There was a problem starting the up.time Web Server (httpd).");
						}
					}
					elseif ($is_scripted) {
						print_msg(LogLevel::INFO, "-----------------------------------------------------------------------------------------------------");
						print_msg(LogLevel::INFO, "WARNING: Plugin may not work properly until the up.time Web Server (httpd) is restarted!");
						print_msg(LogLevel::INFO, "-----------------------------------------------------------------------------------------------------");
					}
				}
			}
			
			
			// copy/move zip to backup dir
			close_file($zip);
			$backup_file_name = $backup_dir . remove_dirs_from_file_name($zip_file);
			$plugin_info->set_file_name($backup_file_name);
			$backup_file_name = $backup_dir . remove_dirs_from_file_name($zip_file);
			if ( copy($zip_file, $backup_file_name) ) {
				print_msg(LogLevel::INFO, "Created backup file in '{$backup_file_name}'");
			}
			else {
				print_msg(LogLevel::ERROR, "ERROR: Could not create backup in '{$backup_file_name}'");
			}
			
			// save the plugin to the DB
			$installed_plugins = new InstalledPlugins();
			$installed_plugins->load($plugins_dir);
/*
print "<pre>";
print_r($plugin_info);
print "</pre>";
*/
			if ($plugin_info->get_sub_type() == PluginSubType::NEWPLUGIN) {
				$installed_plugins->addNewPlugin($plugin_info);
				print_msg(LogLevel::INFO, "Added plugin info to database.");
			}
			elseif ($plugin_info->get_sub_type() == PluginSubType::UPDATE) {
				// get previous installed_date for plugin
				$old_plugin_info = $installed_plugins->getPluginInfo($installed_plugins->getPluginID($plugin_info->get_name()));
				$installed_date = $old_plugin_info['modified_date'];
				$installed_plugins->addNewPlugin($plugin_info, $installed_date);
				print_msg(LogLevel::INFO, "Updated plugin info in database.");
			}
			
			// show readme text (if there was any)
			if (strlen($plugin_info->get_readme()) > 0) {
				$readme = $plugin_info->get_readme();
				print_msg(LogLevel::INFO, "################################# README.txt ########################################");
				print_msg(LogLevel::INFO, $readme);
				print_msg(LogLevel::INFO, "#####################################################################################");
			}
		}
	}
	else {
		// invalid file
		print_msg(LogLevel::ERROR, "File is not supported on this platform.");
		close_file($zip);
		exit(2);
	}
}
else {
	// invalid file
	print_msg(LogLevel::ERROR, "File is not a valid plugin or package file.");
	close_file($zip);
	exit(2);
}

recursiveDelete($tmp_dir);
?>
