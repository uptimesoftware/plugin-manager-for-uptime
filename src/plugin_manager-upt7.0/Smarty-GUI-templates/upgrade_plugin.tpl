{* Smarty *}
<html>
<head>
<link rel="stylesheet" href="uptime.css" type="text/css" />

<script type="text/javascript">
// hide and rename submit button to prevent double-clicks (and look cooler ;) )
function hide_submit_button() {
	document.getElementById('click_once').value = "Installing...";
	document.getElementById('click_once').disabled = true;
	document.getElementById('install_form').submit();
	return true;
}
</script>

</head>
<body>

<div class="TitleBar">Step 1: Uninstalling Old Plug-in...</div>

<font color='red'>{$error_msg}</font><br />

<a href=''>Go Back</a>
<br />
<form action="index.php" enctype="multipart/form-data" method="POST" id="install_form">
<textarea rows='30' cols='120'>
{$output}
</textarea>
<br />
<input type="hidden" name="plugin_on_upgrade_remove_files" value="{$plugin_on_upgrade_remove_files}" />
<input type="hidden" name="plugin_on_upgrade_remove_monitors" value="{$plugin_on_upgrade_remove_monitors}" />
{if $restart_core == "true"}
<input type="hidden" name="restart_core" value="yes" />
{else}
<input type="hidden" name="restart_core" value="no" />
{/if}
<input type="hidden" name="plugin_is_update" value="true" />
<input type="hidden" name="uploaded_full_file_name" value="{$uploaded_full_file_name}" />
<input type="hidden" name="install" value="Step 2: Install New Plug-in" />
<input type="submit" name="install" value="Step 2: Install New Plug-in" id="click_once" onclick="hide_submit_button();">
</form>
<br/><br/>

</body></html>
