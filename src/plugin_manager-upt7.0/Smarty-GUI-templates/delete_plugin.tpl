{* Smarty *}
<html>
<head>
<link rel="stylesheet" href="uptime.css" type="text/css" />

<script type="text/javascript">
// hide and rename submit button to prevent double-clicks (and look cooler ;) )
function hide_submit_button() {
	document.getElementById('click_once').value = "Deleting Plugin...";
	document.getElementById('click_once').disabled = true;
	document.getElementById('click_once_no').disabled = true;
	document.getElementById('retval').name = "yes";
	document.getElementById('retval').value = "Yes";
	document.getElementById('install_form').submit();
	return true;
}
</script>

</head>
<body>
<div class="TitleBar">Deleted Plugin - {$plugin_info['name']}</div>

{if $errors > 0}
<p id="errors"><font color='red'>{$error_msg}</font></p>
{/if}

<form method='POST' id="install_form">
<br />

<table border='0' cellspacing="0" cellpadding="0" id="svStatusList" class='tablehelper'>
<tr class="RowDivider">
<th>Name:</th>
<td>{$plugin_info['name']} (v{$plugin_info['version']})</td>
</tr>
<tr class="RowDivider">
<th>Description:</th>
<td>{$plugin_info['desc']}</td>
</tr>
<tr class="RowDivider">
<th>Installed:</th>
<td>{$plugin_info['install_date']|date_format:"%Y-%m-%d %I:%M:%S %p"}</td>
</tr>
<tr class="RowDivider">
<th>Restart Core:</th>
<td>{$plugin_info['restart_core']}</td>
</tr>
<tr class="RowDivider">
	<th>Restart up.time Core Automatically</th>
{if $plugin_info['restart_core'] == "true"}
	<td><input type="checkbox" name="restart_core" value="yes" checked />Yes</td>
{else}
	<td>No</td>
{/if}
</tr>
</table>
<br />

<p><b>Are you sure you want to uninstall the following plugin?</b><br />
<i>WARNING: All monitors <u>must be deleted in up.time</u> for the uninstall to work!</i></p>
<input type='hidden' name='id' value="{$plugin_info['id']}">
<input type='hidden' id="retval" />
<input type='submit' name='yes' value='Delete' id="click_once" onclick="hide_submit_button();" />
<input type='submit' name='no' value='No' id="click_once_no" />
</form>
<br /><br />

<table border='0' cellspacing="0" cellpadding="0" id="svStatusList" class='tablehelper'>
<tr class="RowDivider">
	<th>
		File(s)
	</th>
</tr>
{section name=num loop=$files step=1}
<tr>
	<td>
		{$files[num]}
	</td>
</tr>
{/section}
</table>

</body></html>
