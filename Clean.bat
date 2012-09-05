@echo Off
del /s /a *.suo *.ncb *.user *.pdb *.netmodule *.aps *.ilk *.sdf 2>nul
FOR /R . %%d IN (.) DO rd /s /q "%%d\x64" 2>nul
FOR /R . %%d IN (.) DO rd /s /q "%%d\Debug" 2>nul
FOR /R . %%d IN (.) DO rd /s /q "%%d\Release" 2>nul
FOR /R . %%d IN (.) DO rd /s /q "%%d\ipch" 2>nul
FOR /R . %%d IN (.) DO rd /s /q "%%d\Obj" 2>nul

rem If the Properties directory is empty, remove it
FOR /R . %%d in (.) do rd /q "%%d\Properties" 2> nul
