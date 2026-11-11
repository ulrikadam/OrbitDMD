# preparer.ps1 - lance en administrateur par l'installateur OrbitDMD
#
# Efface la carte choisie, cree une partition FAT32 "ORBITDMD" (31 Go au plus : le
# Raspberry Pi demarre sur du FAT32, que Windows ne formate pas au-dela de 32 Go),
# puis copie le dossier prepare par l'installateur et verifie la copie.
# Avancement et resultat : lignes ajoutees au fichier $Journal
#   ETAPE <nom>  /  OK <lettre> <nombre de fichiers>  /  ERREUR <message>

param(
	[Parameter(Mandatory = $true)] [int] $Disque,
	[Parameter(Mandatory = $true)] [string] $Modele,
	[Parameter(Mandatory = $true)] [long] $Taille,
	[Parameter(Mandatory = $true)] [string] $Source,
	[Parameter(Mandatory = $true)] [string] $Journal
)

$ErrorActionPreference = 'Stop'
function Ecrire ([string] $Texte) { Add-Content -LiteralPath $Journal -Value $Texte -Encoding UTF8 }

try {
	$d = Get-Disk -Number $Disque

	# memes controles que l'installateur, au cas ou les disques auraient change entre-temps
	if ($d.FriendlyName -ne $Modele -or $d.Size -ne $Taille) {
		throw "La carte a change depuis votre choix. Rien n'a ete modifie : recommencez."
	}
	if ($d.IsSystem -or $d.IsBoot -or @('USB', 'SD', 'MMC') -notcontains [string] $d.BusType) {
		throw "Ce disque n'est pas une carte amovible. Rien n'a ete modifie."
	}
	if ($d.Size -gt 256GB) {
		throw "Disque de plus de 256 Go : refuse par prudence. Rien n'a ete modifie."
	}
	if ($d.IsReadOnly) {
		throw "La carte est protegee en ecriture (petit verrou sur le cote de l'adaptateur SD)."
	}

	Ecrire 'ETAPE effacement'
	if ($d.IsOffline) { Set-Disk -Number $Disque -IsOffline $false }
	if ($d.NumberOfPartitions -gt 0) {
		Clear-Disk -Number $Disque -RemoveData -RemoveOEM -Confirm:$false
	}

	# apres l'effacement, une carte peut etre vierge (RAW) ou rester en MBR/GPT selon le lecteur
	$d = Get-Disk -Number $Disque
	switch ([string] $d.PartitionStyle) {
		'RAW' { Initialize-Disk -Number $Disque -PartitionStyle MBR }
		'GPT' { Set-Disk -Number $Disque -PartitionStyle MBR }
	}
	if ((Get-Disk -Number $Disque).NumberOfPartitions -gt 0) {
		throw "La carte n'a pas pu etre videe. Retirez-la, remettez-la, puis recommencez."
	}

	Ecrire 'ETAPE partition'
	$taillePartition = [Math]::Floor([Math]::Min($d.Size - 16MB, 31GB) / 1MB) * 1MB
	$p = New-Partition -DiskNumber $Disque -Size $taillePartition -MbrType FAT32 -IsActive

	Ecrire 'ETAPE formatage'
	Format-Volume -Partition $p -FileSystem FAT32 -NewFileSystemLabel 'ORBITDMD' -Confirm:$false -Force | Out-Null
	$p | Add-PartitionAccessPath -AssignDriveLetter
	$lettre = $null
	for ($i = 0; $i -lt 20 -and -not $lettre; $i++) {
		Start-Sleep -Milliseconds 500
		$lettre = (Get-Partition -DiskNumber $Disque -PartitionNumber $p.PartitionNumber).DriveLetter
		if ($lettre -eq [char] 0) { $lettre = $null }
	}
	if (-not $lettre) { throw "La carte est formatee mais Windows ne lui a pas donne de lettre." }
	$dest = "${lettre}:\"

	Ecrire 'ETAPE copie'
	robocopy $Source $dest /E /R:2 /W:2 /NFL /NDL /NJH /NJS /NP | Out-Null
	if ($LASTEXITCODE -ge 8) { throw "Erreur pendant la copie (robocopy $LASTEXITCODE)." }

	Ecrire 'ETAPE verification'
	$fichiers = @(Get-ChildItem -LiteralPath $Source -Recurse -File)
	$differences = 0
	foreach ($f in $fichiers) {
		$relatif = $f.FullName.Substring($Source.TrimEnd('\').Length).TrimStart('\')
		$copie = Join-Path $dest $relatif
		if (-not (Test-Path -LiteralPath $copie) -or (Get-Item -LiteralPath $copie).Length -ne $f.Length) { $differences++ }
	}
	if ($differences -gt 0) { throw "$differences fichier(s) mal copie(s) : la carte est peut-etre defectueuse." }

	Ecrire "OK $lettre $($fichiers.Count)"
}
catch {
	Ecrire ("ERREUR " + $_.Exception.Message)
}
