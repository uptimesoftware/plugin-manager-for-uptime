<?php
include_once('plugin_functions.inc');

// PHP.INI CLASS
//
class UptimePhpIniFile {
	// variables
	protected $uptime_dir;
	protected $php_dir = "/apache/php";	// Windows monitoring station
	protected $file_name = "php.ini";
	protected $full_file_name = "/apache/php/php.ini";

	public function get_uptime_dir() { return $this->uptime_dir; }
	protected function set_uptime_dir($val) { $this->uptime_dir = $val; $this->update_full_file_name(); }
	public function get_php_dir() { return $this->php_dir; }
	protected function set_php_dir($val) { $this->php_dir = $val; $this->update_full_file_name(); }
	public function get_file_name() { return $this->file_name; }
	public function set_file_name($val) { $this->file_name = $val; $this->update_full_file_name(); }
	
	protected function update_full_file_name() { $this->full_file_name = fix_directory_slashes( "{$this->uptime_dir}/{$this->php_dir}/{$this->file_name}" ); }
	public function get_full_file_name() { return $this->full_file_name; }


	protected function make_backup() {
		$success = false;
		if ( file_exists($this->get_full_file_name()) ) {
			// let's make 2 backups
			// php.ini.old
			// php.ini.older
			$backup1 = $this->get_full_file_name() . ".old";
			$backup2 = $this->get_full_file_name() . ".older";
			if ( file_exists($backup1) ) {
				if ( file_exists($backup2) ) {
					// delete old backup2
					unlink($backup2);
				}
				// make backup 2
				copy($backup1, $backup2);
				// delete old backup1
				unlink($backup1);
			}
			// make backup 1
			$success = copy($this->get_full_file_name(), $backup1);
			
			if ( ! $success ) {
				// error copying :(
				print("Error: Couldn't copy '" . $this->get_full_file_name() . "' to '{$backup1}'.");
			}
		}
		return $success;
	}
	
	
	
	// updatePhpIni function
	public function updatePhpIni($uptime_dir, $plugin_info) {
		$count = 0;
		$newFileContents = "";
		$this->set_uptime_dir($uptime_dir);
		if (get_os_platform() == 'win') {	// Windows
			// check if file exists
			if (file_exists($this->get_full_file_name())) {
				// read file
				$file = @fopen($this->get_full_file_name(), "r");
				if ($file) {
					// read file line-by-line
					while(!feof($file)) {
						// read the next line
						$line = trim(fgets($file));
						if (preg_match("/^;extension\=.*/i", $line)) {
							// it's an ";extension=..." line, so let's parse it further
							$found = false;
							$extensions = $plugin_info->get_php_extensions();
							foreach ($extensions as $ext) {
								// let's compare each extension to see if it matches
								if (preg_match("/^\;extension\={$ext}\.dll/i", trim($line))) {
									// found the line, now let's uncomment it by removing the ";"
									$line = preg_replace('/\;/', '', $line);
									$count++;
									// exit foreach loop early since we already found the answer
									break;
								}
							}
							// add current line; it will either be unchanged or fixed (great either way)
							$newFileContents .= $line . "\r\n";
						}
						else {
							// keep current line
							$newFileContents .= $line . "\r\n";
						}
					}
					fclose($file);
				}
				
				// let's only make changes to the file if there were any to make
				if ($count > 0) {
					
					// make auto-backup
					$this->make_backup();
					
					// now let's update the file
					// read file
					$file = @fopen($this->get_full_file_name(), "w");
					if ($file) {
						fwrite($file, $newFileContents);
					}
					fclose($file);
				}
			}
		}
		return $count;
	}
	
	
}

?>