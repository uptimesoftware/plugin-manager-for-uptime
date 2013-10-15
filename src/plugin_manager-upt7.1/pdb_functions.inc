<?php

// PLUGIN DATABASE CLASSES
//
// Plugin File class
class InstalledPlugins {
	// variables
	// id, version, install_date, name, description
	protected $info_db_file = "plugins.db";
	protected $files_db_file = "files.db";
	protected $xmls_db_file = "xmls.db";
//	protected $agent_files_db_file = "agent-files.db";
	
	protected $db_dir = "";
	public $installed_plugins;
	public $files = array();
	public $xmls = array();
	
	
	// PRIVATE FUNCTIONS
	protected function create_file($filename) {
		$rv = false;
		$handle = fopen($filename, 'w');
		if ($handle) {
			$rv = true;
		}
		fclose($handle);
		return $rv;
	}
	protected function getNewId() {
		// determine the max ID and add one
		$max_id = 0;
		foreach ($this->installed_plugins as $line) {
			if ($line['id'] > $max_id) {
				$max_id = $line['id'];
			}
		}
		// add one
		$max_id++;
		return $max_id;
	}
	protected function getPluginArrayID($monitor_name) {
		// determine the max ID and add one
		$i = 0;
		foreach ($this->installed_plugins as $line) {
			if ($line['name'] == $monitor_name) {
				return $i;
			}
			$i++;
		}
		return null;
	}
	protected function loadPluginInfo() {
		$ok = true;
		$db_plugins_file = $this->db_dir . $this->info_db_file;
		if (file_exists($db_plugins_file)) {
			$handle = @fopen($db_plugins_file, "r");
			if ($handle) {
				while (($buffer = fgets($handle, 4096)) !== false) {
					$this->parseLine($buffer);
				}
				if (!feof($handle)) {
					//echo "Error reading DB file.\n";
					$ok = false;	// error
				}
				fclose($handle);
			}
			else {
				$ok = false;	// error
			}
		}
		else {
			// create empty DB file(s)
			$ok = $this->create_file($db_plugins_file);
		}

		return $ok;
	}
	protected function loadPluginFiles() {
		$ok = true;
		$db_plugins_file = $this->db_dir . $this->files_db_file;
		$handle = @fopen($db_plugins_file, "r");
		if (file_exists($db_plugins_file)) {
			if ($handle) {
				while (($buffer = fgets($handle, 4096)) !== false) {
					$line = preg_split("/\|/", $buffer);
					array_push($this->files, array('id' => $line[0], 'file' => trim($line[1])));
				}
				if (!feof($handle)) {
					//echo "Error reading DB file.\n";
					$ok = false;	// error
				}
				fclose($handle);
			}
			else {
				$ok = false;	// error
			}
		}
		else {
			// create empty DB file(s)
			$ok = $this->create_file($db_plugins_file);
		}
		return $ok;
	}
	protected function loadPluginXmls() {
		$ok = true;
		$db_plugins_file = $this->db_dir . $this->xmls_db_file;
		$handle = @fopen($db_plugins_file, "r");
		if (file_exists($db_plugins_file)) {
			if ($handle) {
				while (($buffer = fgets($handle, 4096)) !== false) {
					$line = preg_split("/\|/", $buffer);
					array_push($this->xmls, array('id' => $line[0], 'xml' => trim($line[1]), 'monitor' => trim($line[2])));
				}
				if (!feof($handle)) {
					//echo "Error reading DB file.\n";
					$ok = false;	// error
				}
				fclose($handle);
			}
			else {
				$ok = false;	// error
			}
		}
		else {
			// create empty DB file(s)
			$ok = $this->create_file($db_plugins_file);
		}
		return $ok;
	}
	protected function isNewerVersion($old_version, $new_version) {
		// major, minor, build (no revision)
		$limit = 3;
		$old = explode(".", $old_version, $limit);
		$new = explode(".", $new_version, $limit);
		
		if ( count($old) == 2 && count($new) == 2 ) {
			$old_major = intval($old[0]);
			$old_minor = intval($old[1]);
			//$old_build = intval($old[2]);
			$new_major = intval($new[0]);
			$new_minor = intval($new[1]);
			//$new_build = intval($new[2]);
			
			if ($new_major > $old_major) {
				return true;
			}
			elseif ($new_major == $old_major) {
				if ($new_minor > $old_minor) {
					return true;
				}
				// no build numbers in version number
				/*elseif ($new_minor == $old_minor) {
					if ($new_build > $old_build) {
						return true;
					}
				}*/
			}
		}
		
		return false;
	}
	protected function deleteLinesFromFile($id, $ori_file) {
		$rv = true;
		if (is_int($id)) {
			$tmp_db_file = $this->db_dir . "tmp_file." . getmypid();
			$tmp_db_file2 = $this->db_dir . "tmp2_file." . getmypid();
			$handle = @fopen($ori_file, "r");
			$tmp_handle = @fopen($tmp_db_file, "w");
			if (file_exists($ori_file) && file_exists($tmp_db_file)) {
				if ($handle) {
					while (($buffer = fgets($handle, 4096)) !== false) {
						$line = preg_split("/\|/", $buffer);
						// ignore any lines with the ID received
						if ($line[0] != $id) {
							// write to tmp file
							fwrite($tmp_handle, trim($buffer) . "\n");
						}
					}
					if (!feof($handle)) {
						//echo "Error reading DB file.\n";
						$rv = false;
					}
				}
			}
			fclose($tmp_handle);
			fclose($handle);
			// rename original file
			$ok = rename($ori_file, $tmp_db_file2);
			if (! $ok) { $rv = false; }	// something failed
			// rename new one
			$ok = rename($tmp_db_file, $ori_file);
			if (! $ok) { $rv = false; }	// something failed
			// remove original file
			$ok = unlink($tmp_db_file2);
			if (! $ok) { $rv = false; }	// something failed
		}
		return $rv;
	}
	
	
	
	
	
	// PUBLIC FUNCTIONS
	public function parseLine($line) {
		// parse line (from file) and split the variables
		$limit = 8;
		$arr = preg_split("/\|/", trim($line), $limit);
		// file structure:
		// id|version|installdate|restart|name|description
		if (count($arr) == $limit) {
			$id = intval(($arr[0]));
			if (preg_match("/[\d]+\.[\d]+/", trim($arr[1]), $matches) > 0) {
				$version = $matches[0];
			}
			else {
				$version = "0.0";
			}
			// make sure the install_date is in unix time format (all numbers)
			$install_date = intval(trim($arr[2]));
			$modified_date = intval(trim($arr[3]));
			if (trim($arr[4]) == 'true') {
				$restart_core = trim($arr[4]);
				$restart_core_bool = true;
			}
			elseif (trim($arr[4]) == 'false') {
				$restart_core = trim($arr[4]);
				$restart_core_bool = false;
			}
			else {
				$restart_core = 'false';
				$restart_core_bool = false;
			}
			// monitor file name (backup)
			$filename = trim($arr[5]);
			// monitor name
			$name = trim($arr[6]);
			// monitor description
			$desc = trim($arr[7]);
			// put it all into an array
			$arr = array('id' => $id,
					'version' => $version,
					'install_date' => $install_date,
					'modified_date' => $modified_date,
					'restart_core' => $restart_core,
					'restart_core_bool' => $restart_core_bool,
					'name' => $name,
					'desc' => $desc,
					'filename' => $filename);
			array_push($this->installed_plugins, $arr);
		}
	}
	
	public function length() {
		return count($this->installed_plugins);
	}
	
	
	public function getPluginInfo($id) {
		if (is_int($id)) {
			foreach ($this->installed_plugins as $plugin) {
				if ($plugin['id'] == $id) {
					return $plugin;
				}
			}
		}
		return null;
	}
	public function getFiles($id) {
		$rv = array();
		if (is_int($id)) {
			foreach ($this->files as $line) {
				if ($line['id'] == $id) {
					array_push($rv, $line['file']);
				}
			}
		}
		return $rv;
	}
	public function getXmls($id) {
		$rv = array();
		if (is_int($id)) {
			foreach ($this->xmls as $line) {
				if ($line['id'] == $id) {
					$arr = array('xml' => $line['xml'], 'monitor' => $line['monitor']);
					array_push($rv, $arr);
				}
			}
		}
		return $rv;
	}
	
	
	public function getAll() {
		return $this->installed_plugins;
	}
	
	public function load($plugin_dir) {
		$ok = true;	// return value; if true, no errors; false means there was an error
		unset($this->installed_plugins);
		$this->installed_plugins = array();
		$this->db_dir = fix_directory_slashes($plugin_dir . "/db/");
		
		// create directory if it doesn't exist
		if (! file_exists($this->db_dir)) {
			mkdir($this->db_dir, 0777, true) or ( print("Error: Could not create db directory: '{$this->db_dir}'\n") and exit(10) );
		}

		
		// read plugins file(s)
		$ok = $this->loadPluginInfo();
		$ok = $this->loadPluginFiles();
		$ok = $this->loadPluginXmls();
		return $ok;
	}

	public function pluginExists($name) {
		$ok = false;
		foreach ($this->installed_plugins as $plugin) {
			if ($plugin['name'] == $name) {
				$ok = true;
				break;
			}
		}
		return $ok;
	}
	
	public function addNewPlugin($plugin_info, $installed_date = null) {
		$ok = false;	// return value stating whether the plugin was added
		// check if the plugin exists already
		if ( ! $this->pluginExists($plugin_info->get_name()) || $plugin_info->get_sub_type() == PluginSubType::UPDATE) {
			// add plugin info
			$db_plugins_file = $this->db_dir . $this->info_db_file;
			$db_files        = $this->db_dir . $this->files_db_file;
			$db_xmls         = $this->db_dir . $this->xmls_db_file;
			$new_id = $this->getNewId();
			
			if ( ! file_exists($db_plugins_file)) {
				// create empty DB file(s)
				$ok = $this->create_file($db_plugins_file);
			}
			
			// get current time
			$modified_date = time();
			if ( is_null($installed_date) ) {
				$installed_date = $modified_date;
			}
			
			// if we're updating a plugin, let's remove the old line (in plugins.db) and then insert a new one (with the original installed date)
			if ($plugin_info->get_sub_type() == PluginSubType::UPDATE) {
				// get ID of current plugin
				$new_id = $this->getPluginID($plugin_info->get_name());
				$this->deletePlugin($new_id);
			}
			
			$handle = @fopen($db_plugins_file, "a");
			// file structure:
			// id|version|installdate|modifieddate|restart|name|description
			if ($handle) {
				$line = "";
				$line .= $new_id;	// id
				$line .= '|';
				$line .= $plugin_info->get_version();	// version
				$line .= '|';
				$line .= $installed_date;	// install date
				$line .= '|';
				$line .= $modified_date;	// modified date
				$line .= '|';
				$line .= $plugin_info->get_restart_core_string();	// version
				$line .= '|';
				$line .= $plugin_info->get_file_name();	// file name (for backup)
				$line .= '|';
				$line .= $plugin_info->get_name();	// name
				$line .= '|';
				$line .= $plugin_info->get_description();	// desc
				$line .= "\n";
				fwrite($handle, $line);
			}
			fclose($handle);
			
			
			if ( ! file_exists($db_files)) {
				// create empty DB file(s)
				$ok = $this->create_file($db_files);
			}
			$handle = fopen($db_files, "a");
			if ($handle) {
				foreach ($plugin_info->get_files_to_copy() as $file_in_zip) {
					$line = "";
					$line .= $new_id;	// id
					$line .= '|';
					$line .= trim(substr($file_in_zip, strpos($file_in_zip, "/")));	// file name
					$line .= "\n";

					fwrite($handle, $line);
				}
			}
			
			
			if ( ! file_exists($db_xmls)) {
				// create empty DB file(s)
				$ok = $this->create_file($db_xmls);
			}
			$handle = fopen($db_xmls, "a");
			if ($handle) {
				$monitor_names = $plugin_info->get_monitor_names();
				$i = 0;
				foreach ($plugin_info->get_xml_files() as $xml_files) {
					$line = "";
					$line .= $new_id;	// id
					$line .= '|';
					$line .= trim(substr($xml_files, strpos($xml_files, "/")));	// file name
					$line .= '|';
					$line .= trim($monitor_names[$i]);	// monitor name
					$line .= "\n";

					fwrite($handle, $line);
					
					$i++;
				}
			}
			fclose($handle);
		}
		return $ok;
	}

	public function deletePlugin($id, $delete_files_db = true, $delete_xml_db = true, $delete_main_db = true) {
		$rv = true;	// return value - whether the delete(s) were successful or not
		$db_xml_file = $this->db_dir . $this->xmls_db_file;
		$db_files_file = $this->db_dir . $this->files_db_file;
		$db_plugins_file = $this->db_dir . $this->info_db_file;
		
		// delete files info
		if ($delete_files_db) {
			$ok = $this->deleteLinesFromFile($id, $db_xml_file);
			if (! $ok) { $rv = false; }	// something failed
		}
		// delete xml info
		if ($delete_xml_db) {
			$ok = $this->deleteLinesFromFile($id, $db_files_file);
			if (! $ok) { $rv = false; }	// something failed
		}
		// delete plugin info
		if ($delete_main_db) {
			$ok = $this->deleteLinesFromFile($id, $db_plugins_file);
			if (! $ok) { $rv = false; }	// something failed
		}
		return $rv;
	}

	public function checkIfNewer($new_monitor_name, $new_version) {
		$old_id = $this->getPluginArrayID($new_monitor_name);
		$arr = $this->installed_plugins;
		$old_version = $arr[$old_id]['version'];
		
		return $this->isNewerVersion($old_version, $new_version);
	}

	public function getPluginID($monitor_name) {
		// determine the max ID and add one
		foreach ($this->installed_plugins as $line) {
			if ($line['name'] == $monitor_name) {
				return $line['id'];
			}
		}
		return null;
	}

}
?>