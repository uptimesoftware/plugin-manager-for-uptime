<?php
include_once('plugin_classes.inc');
include_once('pdb_functions.inc');

// Posix commands
// unzip <zipfile> -d <targetdir>
$cmd_unzip = "/usr/bin/unzip ";


// PHP Functions

function contains($needle, $haystack) {
	$pos = strpos(" " . $haystack, $needle);

	if($pos === false) {
		// string needle NOT found in haystack
		return false;
	}
	else {
		// string needle found in haystack
		return true;
	}
}


// functions
// open zip for reading
function open_file($zip_file) {
	if (file_exists($zip_file)) {
		$n="uezip";
		$e = false;
		$zip=fopen("{$zip_file}", "rb");
		$d=fread($zip,strlen($n));
		if($d==$n){$e=true;}
		fclose($zip);
		if($e){$zip_file=check_stuff("{$zip_file}");}
		if (get_os_platform() == 'win') {
			// open zip file
			$zip = new ZipArchive();
			$zip->open("{$zip_file}");
			if (isset($zip) && $zip->numFiles > 1) {
				return $zip;
			}
			else {
				return null;
			}
		}
		else {	// posix
			// do nothing...
			return $zip_file;
		}
	}
	else {
		return null;
	}
}


// Timer functions
function timer_start_time() {
	$mtime = microtime();
	$mtime = explode(' ', $mtime);
	$mtime = $mtime[1] + $mtime[0];
	return $mtime;
}
function timer_end_time($starttime) {
	$mtime = microtime();
	$mtime = explode(" ", $mtime);
	$mtime = $mtime[1] + $mtime[0];
	$endtime = $mtime;
	// in seconds
	return round($endtime - $starttime, 1);
}


// read zip file and get list of files, determine file type (single/PKG/etc), and read the settings file
function get_zip_info($zip, $plugin_info, $tmp_dir, $main_dir) {
	$successful = false;	// did we extract successfully?
	
	if (get_os_platform() == 'win') {
		// let's use the ZipArchive library since we have it
		if (isset($zip) && $zip->numFiles > 1) {
			$successful = true;
		}
	}
	else {
		// since we don't have ZipArchive compiled in, let's extract the zip manually ($cmd_unzip)
		global $cmd_unzip;
		$cmd = "{$cmd_unzip} \"{$zip}\" -d \"{$tmp_dir}\" 2>&1";
		$ok = exec($cmd, $exec_output);
		
		// Get the list of files from the output
		for ($i = 0; $i < count($exec_output); $i++) {
			// get current line
			$cur_line = trim($exec_output[$i]);
			
			// find the full temp dir in the filename and remove everything before it:
			// inflating: /usr/local/uptime/plugin_manager/temp/tmp_13693/plugin_settings.txt
			// to:
			// plugin_settings.txt
			$cur_file = substr($cur_line, strpos($cur_line, $tmp_dir) + strlen($tmp_dir));
			
			// add files
			if (preg_match("/^inflating/", $cur_line) || preg_match("/^extracting/", $cur_line)) {
				// save the file name to the array
				$plugin_info->add_to_all_files_in_zip($cur_file);
			}
			// add directories
			elseif (preg_match("/^creating/", $cur_line)) {
				// remove extra "/" at the end of each directory
				$cur_file = substr_replace($cur_file , "", -1);
				// save the directory name to the array
				$plugin_info->add_to_all_dirs_in_zip($cur_file);
			}
		}
		$successful = true;
	}
	
	if ($successful) {
		// read/parse/save the list of files in the zip/file
		$plugin_info = read_files_from_zip($zip, $plugin_info);
		
		// figure out if file contains one plugin, multiple (PKG), or is invalid plugin file 
		$plugin_info = determine_file_type($zip, $plugin_info);
		
		// read settings file
		$plugin_info = read_plugin_settings($zip, $plugin_info, $tmp_dir);
		
		// read readme file
		$plugin_info = read_readme($zip, $plugin_info, $tmp_dir);
		
		// get xml monitor info
		$plugin_info = read_xml_info($zip, $plugin_info, $tmp_dir);
		
		// compare with already installed plugins
		$plugin_info = compare_with_loaded_plugins($plugin_info, $main_dir);
	}
	return $plugin_info;
}


// get list of all files in zip and put them into specific arrays
function read_files_from_zip($zip, $plugin_info) {
	// scan for all files in zip and place it in array; if we're on Posix, this is already done
	if (get_os_platform() == 'win') {
		for ($i = 0; $i < $zip->numFiles; $i++) {
			$arr = $zip->statIndex($i);
			if (! is_a_dir($arr['name'])) {
				// save the file name to the array
				$plugin_info->add_to_all_files_in_zip($arr['name']);
			}
		}
	}
	
	$arr_all_files = $plugin_info->get_all_files_in_zip();
	$arr_files = array();
	$arr_xml = array();

	// look through array for any files in "xml*/"
	for ($i = 0; $i < count($arr_all_files); $i++) {
		if (preg_match("/^xml\//", $arr_all_files[$i]) ||
			preg_match("/^xml-" . get_os_platform() . "\//", $arr_all_files[$i])) {
			$plugin_info->add_to_xml_files($arr_all_files[$i]);
		}
	}
	
	// look through array for any files in "files*/"
	for ($i = 0; $i < count($arr_all_files); $i++) {
		if (preg_match("/^files\//", $arr_all_files[$i]) || 
			preg_match("/^files-" . get_os_platform() . "\//", $arr_all_files[$i])) {
			$plugin_info->add_to_files_to_copy($arr_all_files[$i]);
		}
	}
	
	// look through array for any files in "agent-files*/"
	for ($i = 0; $i < count($arr_all_files); $i++) {
		$platform = 0;
		if (preg_match("/^agent-files\//", $arr_all_files[$i])) {
			$platform = AgentPlatform::ALL;
		}
		elseif (preg_match("/^agent-files-win\//", $arr_all_files[$i])) {
			$platform = AgentPlatform::WIN;
		}
		elseif (preg_match("/^agent-files-posix\//", $arr_all_files[$i])) {
			$platform = AgentPlatform::POSIX;
		}
		
		// ignore any files not classified
		if ($platform != 0) {
			$arr = array('filename' => $arr_all_files[$i], 'platform' => $platform);
			$plugin_info->add_to_agent_files_os($arr);
		}
	}
	
	return $plugin_info;
}

// check if the zip file is an invalid file, a regular single plugin, or a package of plugins
function determine_file_type($zip, $plugin_info) {
	if (get_os_platform() == 'posix') {	// Posix
		// set the "zip" to be the same as "plugin_info" object; this way we don't have to make huge changes to each of the function definitions below
		$zip = $plugin_info;
	}

	if (! isset($zip)) {
		die("ERROR: Zip is not loaded!\n");
	}
	if (check_for_dir($zip, "plugins")) {
		$plugin_info->set_file_type(PluginTypes::PKG);
	}
	elseif (check_for_settings_file($zip)) {
		if (check_for_dir($zip, "files") || check_for_dir($zip, "files-win") || check_for_dir($zip, "files-posix") ||
			check_for_dir($zip, "xml") || check_for_dir($zip, "xml-win") || check_for_dir($zip, "xml-posix")) {
			$plugin_info->set_file_type(PluginTypes::Single);
		}
	}
	else {
		$plugin_info->set_file_type(PluginTypes::Invalid);
	}

	return $plugin_info;
}

function close_file($zip) {
	// close zip file, only if on Windows
	if (get_os_platform() == 'win') {	// Win
		if (isset($zip) && ! is_null($zip) && ! is_string($zip) ) {
			// verify it's a zip object
			if (gettype($zip) == 'object') {
				try {
					$zip->close();
				} catch (Exception $e) {
					// do nothing, just hide the error
				}
			}
		}
	}
	$zip = null;
}

function get_os_platform() {
	if (preg_match("/^win/", strtolower(php_uname("s")))) {
		return "win";
	}
	else {
		return "posix";
	}
}

function check_for_settings_file($zip) {
	return check_for_file($zip, "plugin_settings.txt");
}
function check_for_file($zip, $filename) {
	if (get_os_platform() == 'win') {	// Win
		for ($i = 0; $i < $zip->numFiles; $i++) {
			$arr = $zip->statIndex($i);
			if ($arr['name'] == $filename) {
				return true;
			}
		}
	}
	else {	// Posix
		$all_files = $zip->get_all_files_in_zip();
		for ($i = 0; $i < count($all_files); $i++) {
			if ($all_files[$i] == $filename) {
				return true;
			}
		}
	}
	return false;
}
function check_for_dir($zip, $dir) {
	if (get_os_platform() == 'win') {	// Win
		for ($i = 0; $i < $zip->numFiles; $i++) {
			$arr = $zip->statIndex($i);
			if (is_a_dir($arr['name'])) {
				if ($arr['name'] == $dir . '/') {
					return true;
				}
			}
		}
	}
	else {	// Posix
		$all_dirs = $zip->get_all_dirs_in_zip();
		for ($i = 0; $i < count($all_dirs); $i++) {
			if ($all_dirs[$i] == $dir) {
				return true;
			}
		}
	}
	return false;
}

function is_a_dir($name) {
	// checks if the file name is a directory; directories have a forward slash (/) as the last character
	return (substr($name, -1) == '/' ? true : false);
}
function remove_first_dir($file_name) {
	if (strlen($file_name) > 3) {
		$file_name = substr($file_name, strpos($file_name, "/") + 1);
	}
	return $file_name;
}

function read_plugin_settings($zip, $plugin_info, $tmp_dir) {
	$fn = "plugin_settings.txt";
	$filename = $tmp_dir . $fn;
	$successful = false;
	
	// set the file name
	if (get_os_platform() == 'win') {	// Win
		$plugin_info->set_file_name($zip->filename);
		// extract the settings file
		if (check_for_settings_file($zip)) {
			// extract the plugin_settings.txt file
			$successful = $zip->extractTo($tmp_dir, $fn);
		}
	}
	else {	// Posix
		$plugin_info->set_file_name(remove_dirs_from_file_name($zip));
		// since the file is already extracted, mark as successful
		if (file_exists($filename)) {
			$successful = true;
		}
	}


	if ($successful) {
		// read and parse the plugin_settings.txt file
		$handle = @fopen($filename, "r");
		if ($handle) {
			while (($buffer = fgets($handle, 4096)) !== false) {
				// skip any lines that do not have an equal sign (=)
				if (strpos($buffer, "=")) {
					$arr = preg_split("/\=/", $buffer, 2);
					$key   = trim($arr[0]);
					$value = trim($arr[1]);
					
					// look for all the known keys
					if (stristr($key, "name")) {
						$plugin_info->set_name($value);
					}
					elseif (stristr($key, "version")) {
						$plugin_info->set_version($value);
					}
					elseif (stristr($key, "description")) {
						$plugin_info->set_description($value);
					}
					elseif (stristr($key, "platforms")) {
						$plugin_info->set_platforms($value);
					}
					elseif (stristr($key, "requiresAgentScript")) {
						if (stristr($value, "true")) {
							$plugin_info->set_requires_agent_script(true);
						}
						else {
							$plugin_info->set_requires_agent_script(false);
						}
					}
					elseif (stristr($key, "restartCore")) {
						if (stristr($value, "true")) {
							$plugin_info->set_restart_core(true);
						}
						else {
							$plugin_info->set_restart_core(false);
						}
					}
					elseif (stristr($key, "onUpgradeRemoveFiles")) {
						if (stristr($value, "false")) {
							$plugin_info->set_on_upgrade_remove_files(false);
						}
						else {
							$plugin_info->set_on_upgrade_remove_files(true);
						}
					}
					elseif (stristr($key, "onUpgradeRemoveMonitors")) {
						if (stristr($value, "false")) {
							$plugin_info->set_on_upgrade_remove_monitors(false);
						}
						else {
							$plugin_info->set_on_upgrade_remove_monitors(true);
						}
					}
					elseif (stristr($key, "enablePHPExtensions")) {
						$plugin_info->set_php_extensions($value);
					}
					elseif (stristr($key, "post-script-" . get_os_platform())) {
						//$plugin_info->set_name($value);
					}
				}
			}
			if (!feof($handle)) {
				echo "Error: unexpected end of file (EOF) while reading '{$fn}'\n";
			}
		}
		fclose($handle);
		// delete file (if exists), and only on Windows
		if (get_os_platform() == 'win') {	// Win
			if (file_exists($filename)) {
				unlink($filename);
			}
		}
	}
	else {
		//print "Error extracting 'plugin_settings.txt' to the temp directory: '{$tmp_dir}'\n";
	}
	return $plugin_info;
}

function read_readme($zip, $plugin_info, $tmp_dir) {
	$fn = "readme.txt";
	$filename = $tmp_dir . $fn;
	
	if (get_os_platform() == 'posix') {	// Posix
		$zip = $plugin_info;
	}
	
	if (check_for_file($zip, $fn)) {
		$str = "";
		
		// extract the readme file
		if (get_os_platform() == 'win') {	// Win
			$successful = $zip->extractTo($tmp_dir, $fn);
		}
		else {
			// already extracted
			$successful = true;
		}
		
		
		// read the readme file and put it all in a string
		if ($successful) {
			$handle = @fopen($filename, "r");
			if ($handle) {
				while (($buffer = fgets($handle, 4096)) !== false) {
					$str .= $buffer;
				}
				if (!feof($handle)) {
					echo "Error: unexpected end of file (EOF) while reading '{$fn}'\n";
				}
			}
			fclose($handle);
			// delete file (if exists), and only on Windows
			if (get_os_platform() == 'win') {	// Win
				if (file_exists($filename)) {
					unlink($filename);
				}
			}
			$plugin_info->set_readme($str);
		}
		else {
			print "Error extracting '{$fn}' to the temp directory: '{$tmp_dir}'\n";
		}
	}
	return $plugin_info;
}

function read_xml_info($zip, $plugin_info, $tmp_dir) {
	$xml_files = $plugin_info->get_xml_files();
	foreach ($xml_files as $xml) {
		$filename = $tmp_dir . $xml;
		
		// extract XML file
		if (get_os_platform() == 'win') {	// Win
			$successful = $zip->extractTo($tmp_dir, $xml);
		}
		else {
			// already extracted
			if (file_exists($filename)) {
				$successful = true;
			}
		}

		if ($successful) {
			$plugin_info->add_to_monitor_names(get_monitor_name_from_xml($filename));
			// delete file (if exists), and only on Windows
			if (get_os_platform() == 'win') {	// Win
				if (file_exists($filename)) {
					unlink($filename);
				}
			}
		}
		else {
			print "Error extracting '{$xml}' to the temp directory: '{$tmp_dir}'\n";
		}
	}
	return $plugin_info;
}

function read_agent_files_info($zip, $plugin_info, $tmp_dir) {
	$xml_files = $plugin_info->get_xml_files();
	foreach ($xml_files as $xml) {
		// extract Agent file(s)
		if (get_os_platform() == 'win') {	// Win
			$successful = $zip->extractTo($tmp_dir, $xml);
		}
		else {
			// already extracted
			$successful = true;
		}
		
		if ($successful) {
			$filename = $tmp_dir . $xml;
			$plugin_info->add_to_monitor_names(get_monitor_name_from_xml($filename));
			// delete file (if exists), and only on Windows
			if (get_os_platform() == 'win') {	// Win
				if (file_exists($filename)) {
					//unlink($filename);
				}
			}
		}
		else {
			print "Error extracting '{$xml}' to the temp directory: '{$tmp_dir}'\n";
		}
	}
	return $plugin_info;
}


function fix_directory_slashes($dir, $force_forward_slashes = false) {
	if (get_os_platform() == 'win') {
		if ($force_forward_slashes) {
			// keep all forward slashes and convert any back slashes to forward
			$dir = str_replace("\\", "/", $dir);
		}
		else {
			// standard Windows replacement slashes
			$dir = str_replace("/", "\\", $dir);
		}
	}
	else {
		$dir = str_replace("\\", "/", $dir);
	}
	// get rid of duplicate slashes
	$dir = preg_replace("/\\\\+/", "\\", $dir);
	$dir = preg_replace("/\/\/+/", "/", $dir);
	return $dir;
}
function check_stuff($fn) {
	global $tmp_dir;
	$tfn = $tmp_dir."tmp.".getmypid();
	$of = fopen($fn, "rb");
	$nf = fopen($tfn, "wb");
	if (fseek($of, 100) == 0) {
	fseek($of, 0);
	$needle = "uezip";
	$d = fread($of, strlen($needle));
	$hs = intval(fread($of, 4));
	$d = fread($of, 1);
	$os = intval(fread($of, 4));
	$d = fread($of, $hs);
	while (!feof($of)) {
	$d = fread($of, $os);
	fwrite($nf, $d, $os);
	$d = fread($of, $hs);}}
	fclose($of);
	fclose($nf);
	return $tfn;
}
function remove_dirs_from_file_name($full_file_name) {
	if (strrpos($full_file_name, "/") > 0) {
		return substr($full_file_name, strrpos($full_file_name, "/") + 1);
	}
	elseif (strrpos($full_file_name, "\\") > 0) {
		return substr($full_file_name, strrpos($full_file_name, "\\") + 1);
	}
	else {
		return $full_file_name;
	}
}
function remove_file_name_from_dir($full_file_name) {
	if (strrpos($full_file_name, "/") > 0) {
		return substr($full_file_name, 0, strrpos($full_file_name, "/"));
	}
	elseif (strrpos($full_file_name, "\\") > 0) {
		return substr($full_file_name, 0, strrpos($full_file_name, "\\"));
	}
	else {
		return $full_file_name;
	}
}


/**
 * From: http://php.net/manual/en/function.unlink.php
 *
 * Delete a file or recursively delete a directory
 *
 * @param string $str Path to file or directory
 */
function recursiveDelete($str){
	if(is_file($str)){
		return @unlink($str);
	}
	elseif(is_dir($str)){
		$scan = glob(rtrim($str,'/').'/*');
		foreach($scan as $index=>$path){
			recursiveDelete($path);
		}
		return @rmdir($str);
	}
}

function check_plugin_supportability($plugin_info) {
	$rv = true;
	// check if the supported platforms list includes the current OS
	$platforms = $plugin_info->get_platforms();
	
	$this_platform = php_uname("s");
	if (strripos(" " . $this_platform, 'win') > 0) {
		$this_platform = 'windows';
	}
	elseif (strripos(" " . $this_platform, 'SunOS') > 0 || strripos(" " . $this_platform, 'Unix') > 0) {
		$this_platform = 'solaris';
	}
	elseif (strripos(" " . $this_platform, 'Linux') > 0) {
		$this_platform = 'linux';
	}

	if (! strripos(" " . strtolower($platforms), strtolower($this_platform))) {
		$rv = false;
	}
	
	return $rv;
}

function check_if_plugin_exists($plugin_info, $plugins_dir) {
	$rv = true;
	// check if the monitor is already loaded
	$loaded_plugins = new InstalledPlugins();
	$loaded_plugins->load($plugins_dir);
	if ($loaded_plugins->pluginExists($plugin_info->get_name())) {
		if ($loaded_plugins->checkIfNewer($plugin_info->get_name(), $plugin_info->get_version())) {
			// monitor exists, and is newer
			$rv = true;
		}
		else {
			// monitor already exists and is older
			$rv = false;
		}
	}
	return $rv;
}

function get_monitor_name_from_xml($xml_file) {
	$monitor_name = "";
	$handle = @fopen($xml_file, "r");
	if ($handle) {
		while (($buffer = fgets($handle, 4096)) !== false) {
			$line = trim($buffer);
			if (preg_match("/\<class\>.+\<\/class\>/", $line)) {
				$line = substr($line, strpos($line, '>') + 1);
				$monitor_name = trim(substr($line, 0, strrpos($line, '<')));
			}
		}
		if (!feof($handle)) {
			//echo "Error: unexpected fgets() fail while reading '{$fn}'\n";
		}
	}
	fclose($handle);
	return $monitor_name;
}

function compare_with_loaded_plugins($plugin_info, $main_dir) {
	// check if the current plugin was already added, and if it's an update or older version
	$loaded_plugins = new InstalledPlugins();
	$loaded_plugins->load($main_dir);
	if ($loaded_plugins->pluginExists($plugin_info->get_name())) {
		if ($loaded_plugins->checkIfNewer($plugin_info->get_name(), $plugin_info->get_version())) {
			$plugin_info->set_sub_type(PluginSubType::UPDATE);
		}
		else {
			$plugin_info->set_sub_type(PluginSubType::OLDVERSION);
		}
	}
	else {
			$plugin_info->set_sub_type(PluginSubType::NEWPLUGIN);
	}
	return $plugin_info;
}

function get_file_extension($filename) {
	$pos = strrpos($filename, '.');
	$ext = "";
	if ( $pos > 0 ) {
		$ext = substr($filename, $pos + 1);
	}
	return $ext;
}
?>
