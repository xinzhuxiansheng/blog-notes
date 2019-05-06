**`正文`**

[TOC]

## Windows10系统搜索框空白
打开PowerShell输入下面内容
```shell
Get-AppXPackage -Name Microsoft.Windows.Cortana | Foreach {Add-AppxPackage -DisableDevelopmentMode -Register "$($_.InstallLocation)\AppXManifest.xml"}
```
