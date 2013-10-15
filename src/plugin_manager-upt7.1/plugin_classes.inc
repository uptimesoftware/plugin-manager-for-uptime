<?php
////////////////////////////////////////////////////
// Classes
final class LogLevel
{
	const DEBUG = 0;
	const INFO  = 1;
	const ERROR = 2;
	const FATAL = 3;
	const SILENT = 99;
}

// PluginTypes::Single
final class PluginTypes
{
	const Invalid = 0;
	const Single = 1;
	const PKG = 2;
}


final class AgentPlatform
{
	const WIN   = 1;
	const POSIX = 2;
	const ALL   = 3;
}

final class PluginSubType
{
	const NOTCHECKED = 0;
	const NEWPLUGIN  = 1;
	const UPDATE     = 2;
	const OLDVERSION = 3;
}

// Plugin File class
class PluginFile {
	// variables
	protected $file_name;
	protected $file_type = PluginTypes::Invalid;
	protected $sub_type = PluginSubType::NOTCHECKED;
	protected $name;
	protected $description;
	protected $version;
	protected $platforms;
	protected $requires_agent_script = false;
	protected $restart_core = false;
	protected $on_upgrade_remove_files = true;
	protected $on_upgrade_remove_monitors = false;
	public $php_extensions = array();
	protected $readme = "";
	public $all_files_in_zip = array();
	public $all_dirs_in_zip = array();	// for non-Windows
	public $files_to_copy = array();
	public $xml_files = array();
	public $monitor_names = array();
	public $agent_files = array();	// (filename, platform(win(1), posix(2), all(3)))
	
	function get_file_name() { return $this->file_name; }
	function set_file_name($val) { $this->file_name = $val; }
	function get_file_type() { return $this->file_type; }
	function set_file_type($val) { $this->file_type = $val; }
	function get_sub_type() { return $this->sub_type; }
	function set_sub_type($val) { $this->sub_type = $val; }
	function get_name() { return $this->name; }
	function set_name($val) { $this->name = $val; }
	function get_description() { return $this->description; }
	function set_description($val) { $this->description = $val; }
	function get_version() { return $this->version; }
	function set_version($val) { $this->version = $val; }
	function get_platforms() { return $this->platforms; }
	function set_platforms($val) { $this->platforms = $val; }
	function get_readme() { return $this->readme; }
	function set_readme($val) { $this->readme = $val; }
	function get_on_upgrade_remove_files() { return $this->on_upgrade_remove_files; }
	function get_on_upgrade_remove_files_string() { return ($this->on_upgrade_remove_files? 'true' : 'false'); }
	function set_on_upgrade_remove_files($val) { $this->on_upgrade_remove_files = $val; }
	function get_on_upgrade_remove_monitors() { return $this->on_upgrade_remove_monitors; }
	function get_on_upgrade_remove_monitors_string() { return ($this->on_upgrade_remove_monitors ? 'true' : 'false'); }
	function set_on_upgrade_remove_monitors($val) { $this->on_upgrade_remove_monitors = $val; }
	
	function set_requires_agent_script($val) { $this->requires_agent_script = $val; }
	function get_requires_agent_script() { return $this->get_requires_agent_script; }
	function get_requires_agent_script_string() { return ($this->requires_agent_script ? 'true' : 'false'); }
	function set_restart_core($val) { $this->restart_core = $val; }
	function get_restart_core() { return $this->restart_core; }
	function get_restart_core_string() { return ($this->restart_core ? 'true' : 'false'); }
	
	function get_restart_httpd() { return (count($this->php_extensions) > 0 ? true : false); }
	function get_restart_httpd_string() { return (count($this->php_extensions) > 0 ? 'true' : 'false'); }


	function get_all_files_in_zip() { return $this->all_files_in_zip; }
	function set_all_files_in_zip($val) { $this->all_files_in_zip = $val; }
	function add_to_all_files_in_zip($val) { array_push($this->all_files_in_zip, $val); }

	// PHP Extensions (for php.ini file in up.time 7.1+)
	function get_php_extensions() { return $this->php_extensions; }
	function set_php_extensions($val) {
		// split the string and put the values into the php_extensions array
		$this->php_extensions = preg_split("/\;/", $val);
	}


	// for non-Windows
	function get_all_dirs_in_zip() { return $this->all_dirs_in_zip; }
	function set_all_dirs_in_zip($val) { $this->all_dirs_in_zip = $val; }
	function add_to_all_dirs_in_zip($val) { array_push($this->all_dirs_in_zip, $val); }

	function get_xml_files() { return $this->xml_files; }
	function set_xml_files($val) { $this->xml_files = $val; }
	function add_to_xml_files($val) { array_push($this->xml_files, $val); }

	function get_monitor_names() { return $this->monitor_names; }
	function add_to_monitor_names($val) { array_push($this->monitor_names, $val);}

	function get_agent_files_os() { return $this->agent_files; }
	function add_to_agent_files_os($val) { array_push($this->agent_files, $val);}

	function get_files_to_copy() { return $this->files_to_copy; }
	function set_files_to_copy($val) { $this->files_to_copy = $val; }
	function add_to_files_to_copy($val) { array_push($this->files_to_copy, $val); }
}
?>