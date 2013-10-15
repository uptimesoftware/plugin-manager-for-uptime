{* Smarty *}
<html>
<head>
<link rel="stylesheet" href="uptime.css" type="text/css" />

<script type="text/javascript">
// hide and rename submit button to prevent double-clicks (and look cooler ;) )
function hide_submit_button() {
	javascript:document.getElementById('click_once').value = "Installing...";
	javascript:document.getElementById('click_once').disabled = true;
	document.getElementById('install_form').submit();
	return true;
}
</script>

</head>
<body>
<div class="TitleBar">Package Info</div>
<font color='red'>{$error_msg}</font><br />
{if $plugin_is_update}
<form action="upgrade_plugin.php" enctype="multipart/form-data" method="POST" id="install_form">
{else}
<form enctype="multipart/form-data" method="POST" id="install_form">
{/if}
<table border='0' cellspacing="0" cellpadding="0" id="svStatusList" class='tablehelper'>
<tr class="RowDivider">
	<th>File Name</th>
	<td>{$file_name}</td>
</tr>
<tr>
	<th>Plugin Type</th>
	<td>{$plugin_type}</td>
</tr>
<tr class="RowDivider">
	<th>File Type</th>
	<td>{$file_type}</td>
</tr>
<tr>
	<th>File Size</th>
	<td>{$file_size_kb} KB</td>
</tr>
<tr class="RowDivider">
	<th colspan='2'>&nbsp;<hr width="85%"></th>
</tr>
<tr>
	<th>Name</th>
	<td>{$plugin_name}</td>
</tr>
<tr class="RowDivider">
	<th>Description</th>
	<td>{$plugin_desc}</td>
</tr>
<tr>
	<th>Version</th>
	<td>{$plugin_version}</td>
</tr>
<tr class="RowDivider">
	<th>Supported Platforms</th>
	<td>{$plugin_platforms}</td>
</tr>
<tr>
	<th>Contains Readme?</th>
	<td>{$plugin_readme}</td>
</tr>
<tr>
	<th>Agent Script(s)?</th>
	<td>{$plugin_agent_script}</td>
</tr>
<tr class="RowDivider">
	<th>Requires Restart up.time Core?</th>
	<td>{$plugin_restart_core}</td>
</tr>
{if $plugin_is_update}
<tr>
	<th>Remove Old Files During Upgrade:</th>
	<td><input type="hidden" name="plugin_on_upgrade_remove_files" value="{$plugin_on_upgrade_remove_files}" />{$plugin_on_upgrade_remove_files}</td>
</tr>
<tr class="RowDivider">
	<th>Remove Old Monitors During Upgrade:</th>
	<td><input type="hidden" name="plugin_on_upgrade_remove_monitors" value="{$plugin_on_upgrade_remove_monitors}" />{$plugin_on_upgrade_remove_monitors}</td>
</tr>
{/if}
{if $plugin_restart_core == "true" && $plugin_type != "Invalid plugin"}
<tr>
	<th>Restart up.time Core Automatically</th>
	<td><input type="checkbox" name="restart_core" value="yes" checked />Yes</td>
</tr>
{/if}
<tr>
	<th>&nbsp;</th>
	<td>
		{if strlen($error_msg) > 0}
		<input type="submit" value="Go Back">
		{else}
		<input type="hidden" name="uploaded_full_file_name" value="{$uploaded_full_file_name}" />
		<input type="hidden" name="id" value="{$plugin_id}" />
			{if $plugin_is_new}
			<input type="hidden" name="install" value="Install Package">
			<input type="submit" name="install" value="Install Package" id="click_once" onclick="hide_submit_button();">
			{elseif $plugin_is_update}
			<input type="hidden" name="upgrade_step" value="Step 1: Uninstall Old Plugin">
			<input type="submit" name="upgrade_step" value="Step 1: Uninstall Old Plugin" id="click_once" onclick="hide_submit_button();">
			{else}
			<input type="submit" value="Go Back">
			{/if}
		{/if}
	</td>
</tr>
</table>
</form>
<br /><br />


<table border='0' cellspacing="0" cellpadding="0" id="svStatusList" class='tablehelper'>
<tr class="RowDivider">
	<th>Monitor Files (total: {count($plugin_xmls)})</th>
</tr>
{section name=num loop=$plugin_xmls step=1}
<tr>
	<td>
		<b>{$plugin_monitor_names[num]}</b>  <i>({$plugin_xmls[num]})</i>
	</td>
</tr>
{/section}
</table>
<br /><br />


<table border='0' cellspacing="0" cellpadding="0" id="svStatusList" class='tablehelper'>
<tr class="RowDivider">
	<th>Files (total: {count($plugin_files)})</th>
</tr>
{section name=num loop=$plugin_files step=1}
<tr>
	<td>
		{$plugin_files[num]}
	</td>
</tr>
{/section}
</table>


<br/><br/>
<a href=''>Go Back</a>

</body></html>
