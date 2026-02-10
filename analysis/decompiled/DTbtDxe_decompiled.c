// DTbtDxe.efi - Discrete Thunderbolt DXE Driver
// Lenovo ThinkPad P16 Gen3 (Arrow Lake-S) BIOS N4FET30W (1.11)
// Decompiled and annotated via IDA Pro 9.2 MCP
// Module GUID: 180D765F-C489-4F7A-8A3F-596018499EAF

// ==================== 0x2A0: efi_memset ====================
void *__fastcall sub_2A0(void *a1, unsigned __int64 a2, char a3)
{
  memset(a1, a3, a2);
  return a1;
}

// ==================== 0x300: efi_zeromem ====================
char *__fastcall sub_300(char *a1, unsigned __int64 a2)
{
  memset(a1, 0, 8 * (a2 >> 3));
  memset(&a1[8 * (a2 >> 3)], 0, a2 & 7);
  return a1;
}

// ==================== 0x360: cpu_pause ====================
void sub_360()
{
  _mm_pause();
}

// ==================== 0x370: read_tsc ====================
unsigned __int64 sub_370()
{
  return __rdtsc();
}

// ==================== 0x380: cpuid_wrapper ====================
__int64 __fastcall sub_380(unsigned int a1, _DWORD *a2, _DWORD *a3, _DWORD *a4, _DWORD *a5)
{
  __int64 v11; // [rsp-8h] [rbp-10h]

  v11 = a1;
  __asm { cpuid }
  if ( a4 )
    *a4 = _RCX;
  if ( a2 )
    *a2 = _RAX;
  if ( a3 )
    *a3 = _RBX;
  if ( a5 )
    *a5 = _RDX;
  return v11;
}

// ==================== 0x3D0: _ModuleEntryPoint ====================
EFI_STATUS ModuleEntryPoint(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  __int64 v2; // rcx

  qword_13C0 = (__int64)SystemTable->RuntimeServices;
  qword_13D0 = (__int64)SystemTable->BootServices;
  qword_13C8 = (__int64)SystemTable;
  sub_DA4(ImageHandle);
  sub_DA4(v2);
  return sub_A40();
}

// ==================== 0x408: DTbt_PatchAcpiSsdt ====================
unsigned __int64 sub_408()
{
  unsigned __int64 v0; // rdi
  __int64 i; // rbx
  __int64 v2; // r8
  __int64 v3; // rax
  __int64 v4; // rdi
  unsigned __int64 v5; // rdx
  unsigned __int64 v6; // rax
  unsigned __int64 v7; // rcx
  unsigned __int64 v9; // [rsp+40h] [rbp-30h] BYREF
  unsigned __int64 v10; // [rsp+48h] [rbp-28h] BYREF
  __int64 v11; // [rsp+50h] [rbp-20h] BYREF
  __int64 v12; // [rsp+58h] [rbp-18h] BYREF
  __int64 v13; // [rsp+60h] [rbp-10h] BYREF
  void (__fastcall **v14)(_QWORD, unsigned __int64, _QWORD, __int64 *); // [rsp+68h] [rbp-8h] BYREF
  char v15; // [rsp+90h] [rbp+20h] BYREF
  int v16; // [rsp+98h] [rbp+28h] BYREF
  char v17; // [rsp+A0h] [rbp+30h] BYREF
  __int64 v18; // [rsp+A8h] [rbp+38h] BYREF

  v18 = 0;
  if ( (*(__int64 (__fastcall **)(void *, _QWORD, void (__fastcall ***)(_QWORD, unsigned __int64, _QWORD, __int64 *)))(qword_13D0 + 320))(
         &unk_12F8,
         0,
         &v14) )
  {
    return 0x800000000000000EuLL;
  }
  v0 = 0;
  for ( i = (*(__int64 (__fastcall **)(__int64, void *, _QWORD, unsigned __int64 *, __int64 *))(qword_13D0 + 312))(
              2,
              &unk_12C8,
              0,
              &v10,
              &v11); v0 < v10; ++v0 )
  {
    (*(void (__fastcall **)(_QWORD, void *, __int64 *))(qword_13D0 + 152))(*(_QWORD *)(v11 + 8 * v0), &unk_12C8, &v18);
    v12 = 0;
    v16 = 0;
    i = (*(__int64 (__fastcall **)(__int64, void *, _QWORD, __int64 *, char *, char *, int *))(v18 + 16))(
          v18,
          &unk_1358,
          0,
          &v12,
          &v15,
          &v17,
          &v16);
    if ( !i )
      break;
  }
  (*(void (__fastcall **)(__int64))(qword_13D0 + 72))(v11);
  v3 = v18;
  if ( !v18 )
    return 0x800000000000000EuLL;
  v4 = 0;
  v9 = 0;
  if ( i )
    return i;
  while ( 1 )
  {
    LOBYTE(v2) = 25;
    i = (*(__int64 (__fastcall **)(__int64, void *, __int64, __int64, unsigned __int64 *, __int64 *, int *))(v3 + 24))(
          v3,
          &unk_1358,
          v2,
          v4,
          &v9,
          &v12,
          &v16);
    if ( i < 0 )
      goto LABEL_14;
    v5 = v9;
    if ( *(_QWORD *)(v9 + 16) == 0x7464735374625444LL )
    {
      v6 = v9 + *(unsigned int *)(v9 + 4);
      if ( v9 <= v6 )
        break;
    }
LABEL_13:
    ++v4;
    v9 = 0;
LABEL_14:
    if ( i )
      return i;
    v3 = v18;
  }
  v7 = v9;
  while ( *(_DWORD *)(v7 + 3) != 1447973956 )
  {
    if ( ++v7 > v6 )
      goto LABEL_13;
  }
  *(_DWORD *)(v7 + 9) = qword_13E0;
  *(_WORD *)(v7 + 14) = 42;
  v13 = 0;
  (*v14)(v14, v5, *(unsigned int *)(v5 + 4), &v13);
  return 0;
}

// ==================== 0x5C4: DTbt_ReadyToBootCallback ====================
__int64 __fastcall sub_5C4(__int64 a1)
{
  __int64 v2; // rbx
  __int64 v3; // rax
  __int64 v4; // rax
  _BYTE *v5; // rdi
  __int64 v6; // rsi

  LODWORD(v2) = 0;
  v3 = sub_DA4(a1);
  v4 = sub_E10(&unk_12E8, v3);
  if ( v4 )
  {
    v5 = (_BYTE *)(v4 + 55);
    v6 = 2;
    do
    {
      if ( *(v5 - 27) == 1 && *v5 )
      {
        v2 = v2 & 0xFF0000FF
           | (unsigned __int16)((unsigned __int8)*(v5 - 3) << 8)
           | ((*(v5 - 2) & 0x1F | (32 * (*(v5 - 1) & 7u))) << 16);
        sub_F0C(v2);
      }
      v5 += 32;
      --v6;
    }
    while ( v6 );
  }
  return (*(__int64 (__fastcall **)(__int64))(qword_13D0 + 112))(a1);
}

// ==================== 0x66C: DTbt_PopulatePolicyFromConfig ====================
__int64 sub_66C()
{
  _BYTE *v0; // rdx
  bool v1; // cl
  __int64 result; // rax
  __int64 v3; // [rsp+30h] [rbp+8h] BYREF

  (*(void (__fastcall **)(void *, _QWORD, __int64 *))(qword_13D0 + 320))(&unk_1368, 0, &v3);
  v0 = (_BYTE *)qword_13B0;
  v1 = *(_BYTE *)(qword_13B0 + 28) == 1 || *(_BYTE *)(qword_13B0 + 60) == 1;
  *(_BYTE *)qword_13E0 = v1;
  *(_BYTE *)(qword_13E0 + 35) = (*(_DWORD *)(v3 + 4) & 4) != 0;
  *(_BYTE *)(qword_13E0 + 41) = *(_BYTE *)(v3 + 10) & 1;
  *(_WORD *)(qword_13E0 + 36) = *(_DWORD *)(v3 + 4) >> 3;
  *(_BYTE *)(qword_13E0 + 38) = (*(_DWORD *)(v3 + 4) & 0x80000) != 0;
  *(_WORD *)(qword_13E0 + 39) = *(_WORD *)(v3 + 8);
  *(_BYTE *)(qword_13E0 + 1) = v0[31];
  *(_BYTE *)(qword_13E0 + 2) = v0[29];
  *(_BYTE *)(qword_13E0 + 5) = v0[28];
  *(_BYTE *)(qword_13E0 + 3) = v0[63];
  *(_BYTE *)(qword_13E0 + 4) = v0[61];
  result = qword_13E0;
  *(_BYTE *)(qword_13E0 + 6) = v0[60];
  return result;
}

// ==================== 0x774: DTbt_ProgramPciConfig ====================
void sub_774()
{
  char v0; // r11
  __int64 v1; // r14
  unsigned __int8 v2; // r10
  __int64 v3; // r9
  int v4; // ebx
  int v5; // esi
  int v6; // edi
  __int64 v7; // r8
  unsigned __int64 v8; // rax
  unsigned __int8 v9; // bp
  char v10; // r15
  unsigned __int8 v11; // dl
  __int64 v12; // rcx
  unsigned int v13; // r8d
  __int64 v14; // rbp
  __int64 v15; // rsi
  char v16; // r12
  char v17; // r13
  unsigned __int64 v18; // rcx
  __int64 v19; // r8
  int v20; // edx
  char v21; // r15
  __int64 v22; // rax
  unsigned int v23; // ecx
  int v24; // ecx
  __int64 v25; // rbx
  __int64 v26; // rdi
  __int64 v27; // rdx
  char v28; // [rsp+70h] [rbp+18h] BYREF
  int v29; // [rsp+78h] [rbp+20h]

  v0 = 0;
  v1 = 2;
  if ( qword_13B0 && byte_13B8 )
  {
    v2 = 0;
    v3 = 0;
    do
    {
      if ( *(_BYTE *)(v3 + qword_13B0 + 28) == 1 && *(_BYTE *)(v3 + qword_13B0 + 55) )
      {
        v4 = *(_BYTE *)(v3 + qword_13B0 + 53) & 0x1F;
        v5 = *(unsigned __int8 *)(v3 + qword_13B0 + 52);
        v6 = *(_BYTE *)(v3 + qword_13B0 + 54) & 7;
        v7 = (unsigned int)(32 * v5);
        v8 = (unsigned int)(v5 << 20) | (unsigned __int64)((v6 | (unsigned int)(8 * v4)) << 12);
        v9 = *(_BYTE *)((v8 | 0x19) + 0xC0000000);
        v10 = *(_BYTE *)((v8 | 0x1A) + 0xC0000000);
        v11 = v9;
        if ( (unsigned __int8)(v9 - 1) > 0xFDu )
        {
          v11 = v5 + 40;
          v0 = 1;
          *(_BYTE *)((((v6 | (8 * (v4 | (unsigned int)v7))) << 12) | 0x19LL) + 0xC0000000LL) = v5 + 40;
          *(_BYTE *)((((v6 | (8 * (v4 | (unsigned int)v7))) << 12) | 0x1ALL) + 0xC0000000LL) = v5 + 40;
        }
        v12 = *(unsigned __int16 *)((((unsigned __int64)v11 << 20) | 2) + 0xC0000000);
        if ( v2 )
        {
          if ( v2 == 1 )
            *(_BYTE *)(qword_13E0 + 6) = sub_C98(v12, v2, v7);
        }
        else
        {
          *(_BYTE *)(qword_13E0 + 5) = sub_C98(v12, 0, v7);
        }
        if ( v0 == 1 )
        {
          v13 = (v6 | (8 * (v4 | (unsigned int)v7))) << 12;
          v0 = 0;
          *(_BYTE *)((v13 | 0x19LL) + 0xC0000000LL) = v9;
          *(_BYTE *)((v13 | 0x1ALL) + 0xC0000000LL) = v10;
        }
      }
      ++v2;
      v3 += 32;
    }
    while ( v2 < 2u );
  }
  if ( byte_13B9 && qword_13B0 )
  {
    v14 = 0;
    do
    {
      if ( *(_BYTE *)(qword_13B0 + v14 + 28) == 1 && *(_BYTE *)(qword_13B0 + v14 + 55) )
      {
        v15 = *(unsigned __int8 *)(qword_13B0 + v14 + 52);
        v16 = *(_BYTE *)(qword_13B0 + v14 + 53);
        v17 = *(_BYTE *)(qword_13B0 + v14 + 54);
        v18 = (v17 & 7 | (8 * (v16 & 0x1F | (unsigned __int64)(32 * v15)))) << 12;
        v19 = *(unsigned int *)((v18 | 0x18) + 0xC0000000);
        v20 = *(_DWORD *)((v18 | 0x18) + 0xC0000000) >> 8;
        v29 = *(_DWORD *)((v18 | 0x18) + 0xC0000000);
        if ( (unsigned __int8)(v20 - 1) > 0xFDu )
        {
          LOBYTE(v20) = v15 + 40;
          v21 = 1;
          *(_BYTE *)((v18 | 0x19) + 0xC0000000) = v15 + 40;
          *(_BYTE *)((v18 | 0x1A) + 0xC0000000) = v15 + 40;
        }
        else
        {
          v21 = 0;
        }
        v22 = (unsigned __int8)v20 << 20;
        v23 = *(_DWORD *)(v22 + 3221225472LL);
        if ( (_WORD)v23 == 0x8086 )
        {
          v24 = HIWORD(v23) - 22400;
          if ( !v24 || v24 == 3 )
          {
            v25 = v22 | 0x54C;
            v26 = v22 | 0x548;
            LOBYTE(v19) = 1;
            *(_DWORD *)((v22 | 0x54C) + 0xC0000000LL) = ((unsigned __int8)byte_13B9 << 8) | 0x63;
            sub_CEC(v22 | 0x548, v22, v19, &v28);
            *(_DWORD *)(v25 + 3221225472LL) = 0;
            sub_CEC(v26, v27, 0, &v28);
            LODWORD(v19) = v29;
          }
        }
        if ( v21 )
          *(_DWORD *)((((v17 & 7 | (8 * (v16 & 0x1F | (unsigned __int64)(32 * v15)))) << 12) | 0x18) + 0xC0000000) = v19;
      }
      v14 += 32;
      --v1;
    }
    while ( v1 );
  }
}

// ==================== 0xA40: DTbt_DriverInit ====================
unsigned __int64 __fastcall sub_A40(__int64 a1, __int64 a2)
{
  __int64 v2; // rax
  __int64 v4; // rcx
  char *v5; // rax
  char *v6; // rax
  char *v7; // rbx
  __int64 v8; // rdi
  char v9; // al
  __int64 v10; // [rsp+30h] [rbp-28h] BYREF
  __int64 v11; // [rsp+38h] [rbp-20h] BYREF
  __int64 v12[3]; // [rsp+40h] [rbp-18h] BYREF
  _BYTE *v13; // [rsp+80h] [rbp+28h] BYREF
  __int64 v14; // [rsp+88h] [rbp+30h] BYREF
  char v15; // [rsp+90h] [rbp+38h] BYREF
  __int64 v16; // [rsp+98h] [rbp+40h]

  v14 = a2;
  v11 = 0;
  v16 = 0;
  v13 = 0;
  v2 = sub_DA4(a1);
  qword_13B0 = sub_E10(&unk_12E8, v2);
  if ( !qword_13B0 )
    return 0x800000000000000EuLL;
  v14 = 0xFFFFFFFFLL;
  (*(void (__fastcall **)(__int64, __int64, __int64, __int64 *))(qword_13D0 + 40))(1, 10, 1, &v14);
  qword_13E0 = v14;
  sub_2A0((void *)v14, 0x2Au, 0);
  sub_408();
  sub_66C();
  v10 = 0;
  (*(void (__fastcall **)(__int64 *, void *, __int64 *, _QWORD))(qword_13D0 + 328))(&v10, &unk_1308, &qword_13E0, 0);
  (*(void (__fastcall **)(__int64, __int64, void (*)(), _QWORD, void *, __int64 *))(qword_13D0 + 368))(
    512,
    8,
    sub_774,
    0,
    &unk_1348,
    &v11);
  v12[0] = 3316;
  v5 = (char *)sub_D74(v4, 3316);
  if ( !v5 )
    return 0x8000000000000009uLL;
  v6 = sub_300(v5, 0xCF4u);
  v7 = v6;
  if ( !v6 )
    return 0x8000000000000009uLL;
  v8 = (*(__int64 (__fastcall **)(__int16 *, void *, char *, __int64 *, char *))(qword_13C0 + 72))(
         aSetup,
         &unk_12D8,
         &v15,
         v12,
         v6);
  if ( v8 >= 0 )
  {
    if ( v7[1843] && v7[2001] && v7[2542] && v7[51] )
    {
      v9 = byte_13B8;
      if ( v7[29] )
        v9 = 1;
      byte_13B8 = v9;
    }
    byte_13B9 = (v7[2830] == 2) + 1;
    v8 = (*(__int64 (__fastcall **)(void *, _QWORD, _BYTE **))(qword_13D0 + 320))(&unk_1368, 0, &v13);
    if ( v8 >= 0 && *v13 == 1 )
    {
      v8 = (*(__int64 (__fastcall **)(__int64, __int64, __int64 (__fastcall *)(__int64)))(qword_13D0 + 368))(
             512,
             8,
             sub_5C4);
      if ( v8 >= 0 )
      {
        v14 = 0;
        (*(void (__fastcall **)(__int64 *, void *, __int64 (__fastcall **)(), _QWORD))(qword_13D0 + 328))(
          &v14,
          &unk_1338,
          &off_1378,
          0);
      }
      else
      {
        (*(void (__fastcall **)(__int64))(qword_13D0 + 112))(v16);
      }
    }
  }
  return v8;
}

// ==================== 0xC98: DTbt_IsKnownTbtDeviceId ====================
bool __fastcall sub_C98(unsigned __int16 a1)
{
  if ( a1 > 0x15D3u )
  {
    if ( a1 == 5594 || a1 == 5607 || a1 == 5610 || a1 == 22400 )
      return 1;
    return a1 == 22403;
  }
  else
  {
    if ( a1 == 5587 || a1 == 4403 || a1 == 4406 || a1 == 5494 || a1 == 5496 )
      return 1;
    return a1 == 5568;
  }
}

// ==================== 0xCEC: DTbt_PollPciRegister ====================
char __fastcall sub_CEC(unsigned __int64 a1, __int64 a2, char a3, int *a4)
{
  char v4; // bl
  unsigned int v7; // esi
  int *v8; // rdi
  int v9; // ecx

  v4 = 0;
  v7 = 0;
  v8 = (int *)(((unsigned int)a1 | (a1 >> 4) & 0x10000000) + 3221225472u);
  do
  {
    v9 = *v8;
    *a4 = *v8;
    if ( v9 == -1 )
      break;
    if ( !a3 )
      LOBYTE(v9) = ~(_BYTE)v9;
    if ( (v9 & 1) != 0 )
      return 1;
    sub_1140();
    ++v7;
  }
  while ( v7 < 0x1F4 );
  return v4;
}

// ==================== 0xD74: DTbt_AllocatePool ====================
__int64 __fastcall sub_D74(__int64 a1, __int64 a2)
{
  __int64 v2; // rax
  __int64 v3; // rcx
  __int64 v5; // [rsp+40h] [rbp+18h] BYREF

  v2 = (*(__int64 (__fastcall **)(__int64, __int64, __int64 *))(qword_13D0 + 64))(4, a2, &v5);
  v3 = v5;
  if ( v2 < 0 )
    return 0;
  return v3;
}

// ==================== 0xDA4: DTbt_GetHobData ====================
__int64 sub_DA4()
{
  __int64 v0; // rcx
  __int64 v1; // rax
  unsigned __int64 v2; // r8
  _QWORD *v3; // r9
  _QWORD *i; // rdx

  v0 = qword_13D8;
  v1 = 0;
  if ( !qword_13D8 )
  {
    v0 = 0;
    qword_13D8 = 0;
    v2 = *(_QWORD *)(qword_13C8 + 104);
    if ( v2 )
    {
      v3 = *(_QWORD **)(qword_13C8 + 112);
      for ( i = v3; qword_1318 != *i || qword_1320 != i[1]; i += 3 )
      {
        if ( ++v1 >= v2 )
          return v0;
      }
      v0 = v3[3 * v1 + 2];
      qword_13D8 = v0;
    }
  }
  return v0;
}

// ==================== 0xE10: DTbt_FindHobByGuid ====================
__int64 __fastcall sub_E10(_QWORD *a1, __int64 a2)
{
  __int64 result; // rax

  while ( *(_WORD *)a2 != 0xFFFF )
  {
    if ( *(_WORD *)a2 == 4 )
    {
      result = a2;
      if ( !a2 || *a1 == *(_QWORD *)(a2 + 8) && a1[1] == *(_QWORD *)(a2 + 16) )
        return result;
      a2 = *(unsigned __int16 *)(a2 + 2);
    }
    else
    {
      result = *(unsigned __int16 *)(a2 + 2);
    }
    a2 += result;
  }
  return 0;
}

// ==================== 0xE58: DTbt_BuildPciAddress ====================
unsigned __int64 __fastcall sub_E58(unsigned int a1)
{
  if ( (_BYTE)a1 )
    return ((unsigned __int16)(a1 & 0xFF00) << 12)
         | ((a1 & 0x1F0000 | (unsigned __int64)((a1 >> 8) & 0xE000)) >> 1)
         | ((unsigned __int64)(unsigned __int8)a1 << 32);
  else
    return ((unsigned __int16)(a1 & 0xFF00) << 12) | ((a1 & 0x1F0000 | (unsigned __int64)((a1 >> 8) & 0xE000)) >> 1);
}

// ==================== 0xE98: DTbt_ClearPciMasterAbort ====================
__int16 __fastcall sub_E98(unsigned int a1)
{
  unsigned __int64 v1; // rdx
  __int16 result; // ax

  if ( (_BYTE)a1 )
    v1 = ((unsigned __int16)(a1 & 0xFF00) << 12)
       | ((a1 & 0x1F0000 | (unsigned __int64)((a1 >> 8) & 0xE000)) >> 1)
       | 4
       | ((unsigned __int64)a1 << 32);
  else
    v1 = ((unsigned __int16)(a1 & 0xFF00) << 12) | ((a1 & 0x1F0000 | (unsigned __int64)((a1 >> 8) & 0xE000)) >> 1) | 4;
  result = *(_WORD *)(((unsigned int)v1 | (v1 >> 4) & 0x10000000) + 0xC0000000);
  if ( (result & 4) != 0 )
  {
    result = *(_WORD *)(((unsigned int)v1 | (v1 >> 4) & 0x10000000) + 0xC0000000) & 0xFFFB;
    *(_WORD *)(((unsigned int)v1 | (v1 >> 4) & 0x10000000) + 0xC0000000) = result;
  }
  return result;
}

// ==================== 0xF0C: DTbt_EnumeratePciBus ====================
char __fastcall sub_F0C(unsigned int a1)
{
  unsigned __int64 v1; // rdi
  unsigned __int64 v2; // r9
  unsigned __int64 v3; // rax
  int v4; // esi
  char v5; // al
  __int64 v6; // rbx
  unsigned __int64 v7; // rax
  unsigned __int64 v8; // rcx
  unsigned int v9; // edx
  unsigned int v10; // eax
  int v11; // eax

  v1 = a1;
  v2 = sub_E58(a1);
  LODWORD(v3) = *(_DWORD *)(((unsigned int)v2 | (v2 >> 4) & 0x10000000) + 0xC0000000);
  if ( (_DWORD)v3 && (_DWORD)v3 != 255 )
  {
    if ( (unsigned int)v1 >= 0x1000000 )
    {
      v5 = (unsigned __int8)*(_WORD *)((((unsigned int)v2 + (unsigned int)(v1 >> 24) + 2)
                                      | ((v2 + (v1 >> 24) + 2) >> 4) & 0x10000000)
                                     + 0xC0000000) >> 4;
      if ( v5 == 5 )
      {
        v4 = 2;
      }
      else if ( ((v5 - 4) & 0xFD) != 0 )
      {
        v4 = 1;
        sub_E98(v1);
      }
      else
      {
        v4 = 3;
      }
    }
    else
    {
      v4 = 0;
    }
    LOBYTE(v3) = *(_BYTE *)(((unsigned int)(v2 + 11) | ((v2 + 11) >> 4) & 0x10000000) + 0xC0000000);
    if ( (_BYTE)v3 == 6 )
    {
      LODWORD(v3) = *(_DWORD *)(((unsigned int)(v2 + 24) | ((v2 + 24) >> 4) & 0x10000000) + 0xC0000000) >> 8;
      LODWORD(v6) = (unsigned __int8)v1
                  | (((unsigned __int8)BYTE1(*(_DWORD *)(((unsigned int)(v2 + 24) | ((v2 + 24) >> 4) & 0x10000000)
                                                       + 0xC0000000))
                    | 0xFF00) << 8);
      if ( BYTE1(*(_DWORD *)(((unsigned int)(v2 + 24) | ((v2 + 24) >> 4) & 0x10000000) + 0xC0000000)) )
      {
        while ( 1 )
        {
          if ( v4 == 1 )
            return v3;
          v7 = (_BYTE)v6
             ? ((unsigned __int16)(v6 & 0xFF00) << 12) | ((unsigned __int64)(unsigned __int8)v6 << 32)
             : (unsigned __int16)(v6 & 0xFF00) << 12;
          v8 = v7 | ((unsigned int)v6 >> 1) & 0xF8000;
          v9 = WORD1(v6) & 0x1F;
          v10 = ((unsigned int)v6 >> 21) & 7;
          if ( v9 == 31 )
            break;
          if ( v10 != 7 )
            goto LABEL_20;
LABEL_23:
          v11 = 31;
          if ( v4 != 2 )
            v11 = 0;
          if ( v9 == v11 )
            goto LABEL_31;
          LODWORD(v6) = v6 & 0xFF00FFFF | (v6 + 0x10000) & 0x1F0000;
LABEL_27:
          v3 = sub_E58(v6);
          if ( *(_WORD *)(((unsigned int)v3 | (v3 >> 4) & 0x10000000) + 0xC0000000) != 0xFFFF )
          {
            LOBYTE(v3) = sub_11F8(v3);
            if ( (_BYTE)v3 )
            {
              v6 = v6 & 0xFFFFFF | ((unsigned __int8)v3 << 24);
              LOBYTE(v3) = sub_F0C(v6);
            }
          }
        }
        if ( v10 == 7 )
        {
          LODWORD(v6) = v6 & 0xFF00FFFF;
          goto LABEL_27;
        }
LABEL_20:
        if ( v10 || *(char *)(((unsigned int)(v8 + 14) | ((v8 + 14) >> 4) & 0x10000000) + 0xC0000000) < -1 )
        {
          LODWORD(v6) = (v6 ^ (v6 + 0x200000)) & 0xE00000 ^ v6;
          goto LABEL_27;
        }
        goto LABEL_23;
      }
    }
    if ( v4 != 1 )
LABEL_31:
      LOBYTE(v3) = sub_E98(v1);
  }
  return v3;
}

// ==================== 0x1140: DTbt_Stall1ms ====================
__int64 sub_1140()
{
  unsigned __int64 v0; // rbx
  unsigned __int64 v1; // rdi

  v0 = 1000 * sub_1194();
  v1 = v0 / 0xF4240 + sub_370();
  while ( sub_370() <= v1 )
    sub_360();
  return 1000;
}

// ==================== 0x1194: DTbt_GetTscFreqMhz ====================
unsigned __int64 sub_1194()
{
  __int64 v0; // rbx
  __int64 v1; // rcx
  unsigned int v3; // [rsp+40h] [rbp+8h] BYREF
  unsigned int v4; // [rsp+48h] [rbp+10h] BYREF
  unsigned int v5; // [rsp+50h] [rbp+18h] BYREF

  v0 = 0;
  sub_380(0x15u, &v4, &v5, &v3, 0);
  if ( v4 && v5 )
  {
    v1 = 24000000;
    if ( v3 )
      v1 = v3;
    return (((unsigned __int64)v4 >> 1) + v1 * v5) / v4;
  }
  return v0;
}

// ==================== 0x11F8: DTbt_FindPciExpressCapability ====================
char __fastcall sub_11F8(__int64 a1)
{
  unsigned __int16 v2; // r9
  char v3; // al
  __int64 v4; // rcx
  __int16 v5; // r8

  if ( (*(_BYTE *)(((unsigned int)(a1 + 6) | ((unsigned __int64)(a1 + 6) >> 4) & 0x10000000) + 0xC0000000) & 0x10) != 0 )
  {
    LOBYTE(v2) = 0;
    v3 = *(_BYTE *)(((unsigned int)(a1 + 14) | ((unsigned __int64)(a1 + 14) >> 4) & 0x10000000) + 0xC0000000);
    v4 = 20;
    if ( (v3 & 0x7F) != 2 )
      v4 = 52;
    LOBYTE(v5) = *(_BYTE *)(((unsigned int)(a1 + v4) | ((unsigned __int64)(a1 + v4) >> 4) & 0x10000000) + 0xC0000000)
               & 0xFC;
    if ( (_BYTE)v5 )
    {
      while ( (_BYTE)v2 != 0xFF )
      {
        v2 = *(_WORD *)((((unsigned int)a1 + (unsigned __int8)v5)
                       | ((a1 + (unsigned __int64)(unsigned __int8)v5) >> 4) & 0x10000000)
                      + 0xC0000000);
        if ( (_BYTE)v2 == 16 )
        {
          if ( (unsigned __int8)v5 <= 0x3Fu )
            return 0;
          return v5;
        }
        v5 = HIBYTE(v2);
        if ( !HIBYTE(v2) )
          return 0;
      }
    }
  }
  return 0;
}

