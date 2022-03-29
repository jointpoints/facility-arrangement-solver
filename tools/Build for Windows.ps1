function findPath
{
	param
	(
		[string]$Root,
		[string]$PathPattern,
		[uint16]$CutOff
	)

	Get-ChildItem -Path $Root -Recurse | Where-Object { $_.FullName -match $PathPattern } | ForEach-Object { $_.FullName.Substring(0, $_.FullName.Length - $CutOff) }
}





findPath -Root 'D:\\' -PathPattern '.*\\cplex\\include\\ilcplex\\ilocplex.h' -CutOff 19
