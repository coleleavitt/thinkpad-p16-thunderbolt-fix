// ITbtDxe.efi - Integrated Thunderbolt DXE Driver
// Lenovo ThinkPad P16 Gen3 (Arrow Lake-S) BIOS N4FET30W (1.11)
// Decompiled and annotated via IDA Pro 9.2 MCP

// ==================== 0x280: sub_280 ====================
void *__fastcall sub_280(void *a1, unsigned __int64 a2, char a3)
{
  memset(a1, a3, a2);
  return a1;
}

// ==================== 0x2A0: sub_2A0 ====================
char *__fastcall sub_2A0(char *a1, char *a2, unsigned __int64 a3)
{
  char *result; // rax
  unsigned __int64 v5; // rcx
  char *v6; // rdi
  char *v7; // rsi

  result = a1;
  if ( a2 < a1 && &a2[a3 - 1] >= a1 )
  {
    v7 = &a2[a3 - 1];
    v6 = &a1[a3 - 1];
  }
  else
  {
    v5 = a3;
    a3 &= 7u;
    v5 >>= 3;
    qmemcpy(a1, a2, 8 * v5);
    v7 = &a2[8 * v5];
    v6 = &a1[8 * v5];
  }
  qmemcpy(v6, v7, a3);
  return result;
}

// ==================== 0x340: sub_340 ====================
unsigned __int64 __fastcall sub_340(_BYTE *a1, _BYTE *a2, __int64 a3)
{
  bool v6; // zf

  do
  {
    if ( !a3 )
      break;
    v6 = *a1++ == *a2++;
    --a3;
  }
  while ( v6 );
  return (unsigned __int8)*(a1 - 1) - (unsigned __int64)(unsigned __int8)*(a2 - 1);
}

// ==================== 0x360: _ModuleEntryPoint ====================
EFI_STATUS ModuleEntryPoint(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  __int64 v2; // rax
  __int64 v3; // rcx
  __int64 v4; // rdx
  unsigned int v5; // r10d
  __int64 v6; // r9
  unsigned __int64 i; // r8
  bool v8; // al

  qword_10F8 = (__int64)SystemTable->BootServices;
  qword_10F0 = (__int64)SystemTable;
  sub_924(ImageHandle);
  sub_990(&unk_1020);
  v2 = sub_990(&unk_1000);
  v4 = v2;
  if ( v2 && (*(_BYTE *)(v2 + 24) & 1) != 0 )
  {
    v5 = *(_DWORD *)(v2 + 44);
    v6 = 0;
    v3 = 0;
    for ( i = 0; i < 0x14; i += 5LL )
    {
      byte_10B0[i] = 0;
      byte_10B0[i + 3] = (v5 >> v3) & 7;
      v8 = *(_BYTE *)(v4 + v6++ + 48) != 0;
      byte_10B0[i + 4] = v8;
      v3 += 3;
    }
    byte_1118 = 1;
  }
  return sub_5A4(v3, v4);
}

// ==================== 0x3F8: sub_3F8 ====================
__int64 __fastcall sub_3F8(__int64 a1)
{
  unsigned __int64 v1; // rbx
  unsigned __int8 *v2; // rdi
  unsigned __int64 v3; // rsi
  char v5; // bp
  __int64 v6; // r14
  unsigned __int64 v7; // rdx
  int v8; // r8d
  __int64 v9; // rax

  LODWORD(v1) = 0;
  v2 = byte_10B0;
  v3 = 0;
  v5 = 0;
  v6 = 4;
  do
  {
    if ( v3 >= 4 )
      goto LABEL_10;
    if ( byte_1118 == 1 )
    {
      if ( v2[4] != 1 )
        goto LABEL_10;
      LOBYTE(v7) = v2[3];
      v8 = *v2;
      goto LABEL_9;
    }
    v9 = sub_990(&unk_1000);
    if ( v9 && *(_BYTE *)(v3 + v9 + 48) )
    {
      v7 = (unsigned __int64)*(unsigned int *)(v9 + 44) >> v5;
      v8 = 0;
LABEL_9:
      v1 = v8 | v1 & 0xFF000000 | ((v2[1] | (((32 * (v7 & 7)) | v2[2] & 0x1F) << 8)) << 8);
      sub_CB8(v1);
    }
LABEL_10:
    ++v3;
    v2 += 5;
    v5 += 3;
    --v6;
  }
  while ( v6 );
  return (*(__int64 (__fastcall **)(__int64))(qword_10F8 + 112))(a1);
}

// ==================== 0x4D4: sub_4D4 ====================
__int64 sub_4D4()
{
  __int64 result; // rax
  int v1; // [rsp+40h] [rbp+18h] BYREF
  __int64 v2; // [rsp+48h] [rbp+20h] BYREF

  result = (*(__int64 (__fastcall **)(void *, _QWORD, __int64 *))(qword_10F8 + 320))(&unk_FC0, 0, &v2);
  if ( result >= 0 )
  {
    result = (*(__int64 (__fastcall **)(int *))(v2 + 64))(&v1);
    if ( !v1 )
    {
      result = sub_990(&unk_FD0);
      if ( result )
      {
        result = *(_DWORD *)(result + 52) & 0xF000;
        if ( (_DWORD)result == 4096 )
        {
          result = qword_1120;
          *(_BYTE *)(qword_1120 + 17) = 1;
        }
      }
    }
  }
  return result;
}

// ==================== 0x53C: sub_53C ====================
__int64 __fastcall sub_53C(__int64 a1, int a2)
{
  int v2; // edx
  _QWORD v4[3]; // [rsp+30h] [rbp-18h] BYREF
  __int16 v5; // [rsp+60h] [rbp+18h] BYREF
  int v6; // [rsp+68h] [rbp+20h] BYREF

  v4[0] = 0x7464735373736354LL;
  v6 = qword_1120;
  v5 = 20;
  sub_AEC((unsigned int)v4, a2, 1112429641, (unsigned int)&v6, 4);
  return sub_AEC((unsigned int)v4, v2, 1280201801, (unsigned int)&v5, 2);
}

// ==================== 0x5A4: sub_5A4 ====================
__int64 __fastcall sub_5A4(__int64 a1, __int64 a2)
{
  __int64 v2; // rbx
  __int16 v3; // dx
  __int64 v4; // r8
  char v5; // cl
  __int64 v6; // rdx
  __int64 v7; // rdi
  __int64 v8; // rbx
  __int64 v9; // r9
  unsigned __int8 v10; // cl
  __int64 v11; // rdx
  char *v12; // r8
  char v13; // cl
  __int64 i; // rdx
  __int64 v15; // rdx
  __int64 v17; // [rsp+30h] [rbp-38h] BYREF
  __int64 v18; // [rsp+38h] [rbp-30h] BYREF
  __int64 v19; // [rsp+40h] [rbp-28h] BYREF
  __int64 v20; // [rsp+48h] [rbp-20h] BYREF
  _BYTE v21[24]; // [rsp+50h] [rbp-18h] BYREF
  __int64 v22; // [rsp+90h] [rbp+28h] BYREF
  __int64 v23; // [rsp+98h] [rbp+30h] BYREF
  __int64 v24; // [rsp+A0h] [rbp+38h] BYREF
  __int64 v25; // [rsp+A8h] [rbp+40h] BYREF

  v23 = a2;
  v22 = a1;
  v2 = 0;
  v3 = 0x2000;
  v20 = 0;
  v18 = 0;
  v17 = 0;
  v25 = 0;
  v4 = 2;
  v5 = 0;
  do
  {
    if ( *(_WORD *)((v3 & 0x7000 | 0x68000LL) + 0xC0000000LL) != 0xFFFF )
      v5 = 1;
    v3 += 4096;
    --v4;
  }
  while ( v4 );
  if ( v5 )
  {
    v22 = 0;
    v24 = 0;
    (*(void (__fastcall **)(void *, _QWORD, __int64 *))(qword_10F8 + 320))(&unk_1070, 0, &v22);
    sub_8AC(v22, v6, &v24);
    v7 = sub_990(&unk_1000);
    v8 = sub_990(&unk_1010);
    v23 = 0xFFFFFFFFLL;
    (*(void (__fastcall **)(__int64, __int64, __int64, __int64 *))(qword_10F8 + 40))(1, 10, 1, &v23);
    qword_1120 = v23;
    sub_280((void *)v23, 0x14u, 0);
    v9 = v24;
    *(_BYTE *)(v23 + 8) = *(_BYTE *)(v24 + 100);
    *(_BYTE *)(v23 + 9) = *(_BYTE *)(v9 + 32);
    *(_WORD *)(v23 + 10) = *(_WORD *)(v9 + 34);
    *(_WORD *)(v23 + 18) = *(_WORD *)(v9 + 102);
    if ( v8 )
    {
      v10 = 0;
      while ( *(_BYTE *)(v8 + 4LL * v10 + 24) != 1 )
      {
        if ( ++v10 >= 4u )
          goto LABEL_12;
      }
      *(_BYTE *)v23 = 1;
LABEL_12:
      v11 = 0;
      v12 = (char *)(v8 + 24);
      do
      {
        v13 = *v12;
        v12 += 4;
        *(_BYTE *)(v11 + v23 + 4) = v13;
        ++v11;
      }
      while ( v11 < 4 );
      *(_WORD *)(v23 + 13) = *(_WORD *)(v8 + 40);
      *(_WORD *)(v23 + 15) = *(_WORD *)(v8 + 42);
    }
    if ( v7 )
    {
      for ( i = 0; i < 2; ++i )
        *(_BYTE *)(i + v23 + 1) = *(_BYTE *)(v7 + i + 54);
      *(_BYTE *)(v23 + 12) = (*(_DWORD *)(v7 + 36) >> 30) & (*(int *)(v7 + 36) < 0);
    }
    if ( *(_BYTE *)(v9 + 33) == 1 )
      (*(void (__fastcall **)(__int64, __int64, __int64 (*)(), _QWORD, void *, _BYTE *))(qword_10F8 + 368))(
        512,
        8,
        sub_4D4,
        0,
        &unk_1080,
        v21);
    v19 = 0;
    (*(void (__fastcall **)(__int64 *, void *, __int64 *, _QWORD))(qword_10F8 + 328))(&v19, &unk_FE0, &qword_1120, 0);
    (*(void (__fastcall **)(__int64, __int64, __int64 (__fastcall *)(__int64, int), _QWORD, void *, __int64 *))(qword_10F8 + 368))(
      512,
      8,
      sub_53C,
      0,
      &unk_1080,
      &v20);
    v2 = (*(__int64 (__fastcall **)(void *, _QWORD, __int64 *))(qword_10F8 + 320))(&unk_1070, 0, &v25);
    if ( v2 >= 0 )
    {
      v2 = sub_8AC(v25, v15, &v17);
      if ( v2 >= 0 && *(_BYTE *)(v17 + 28) == 1 )
      {
        v2 = (*(__int64 (__fastcall **)(__int64, __int64, __int64 (__fastcall *)(__int64), _QWORD, void *, __int64 *))(qword_10F8 + 368))(
               512,
               8,
               sub_3F8,
               0,
               &unk_1060,
               &v18);
        if ( v2 >= 0 )
        {
          v23 = 0;
          (*(void (__fastcall **)(__int64 *, void *, __int64 (__fastcall **)(), _QWORD))(qword_10F8 + 328))(
            &v23,
            &unk_FF0,
            &off_10A0,
            0);
        }
        else
        {
          (*(void (__fastcall **)(__int64))(qword_10F8 + 112))(v18);
        }
      }
    }
  }
  return v2;
}

// ==================== 0x8AC: sub_8AC ====================
unsigned __int64 __fastcall sub_8AC(__int64 a1, __int64 a2, __int64 *a3)
{
  unsigned int v3; // edx
  unsigned __int16 v5; // r10
  unsigned int v6; // r11d
  __int64 v7; // rcx

  v3 = 0;
  v5 = 0;
  if ( *(_WORD *)(a1 + 30) )
  {
    v6 = *(unsigned __int16 *)(a1 + 2);
    while ( v3 + 36 <= v6 )
    {
      v7 = v3 + a1 + 36;
      if ( *(_QWORD *)(v7 + 8) == qword_1090 && *(_QWORD *)(v7 + 16) == qword_1098 )
      {
        *a3 = v7;
        return 0;
      }
      ++v5;
      v3 += *(unsigned __int16 *)(v7 + 2);
      if ( v5 >= *(_WORD *)(a1 + 30) )
        return 0x800000000000000EuLL;
    }
  }
  return 0x800000000000000EuLL;
}

// ==================== 0x924: sub_924 ====================
__int64 sub_924()
{
  __int64 v0; // rcx
  __int64 v1; // rax
  unsigned __int64 v2; // r8
  _QWORD *v3; // r9
  _QWORD *i; // rdx

  v0 = qword_1100;
  v1 = 0;
  if ( !qword_1100 )
  {
    v0 = 0;
    qword_1100 = 0;
    v2 = *(_QWORD *)(qword_10F0 + 104);
    if ( v2 )
    {
      v3 = *(_QWORD **)(qword_10F0 + 112);
      for ( i = v3; qword_1050 != *i || qword_1058 != i[1]; i += 3 )
      {
        if ( ++v1 >= v2 )
          return v0;
      }
      v0 = v3[3 * v1 + 2];
      qword_1100 = v0;
    }
  }
  return v0;
}

// ==================== 0x990: sub_990 ====================
__int64 __fastcall sub_990(_QWORD *a1)
{
  __int64 i; // rdx
  __int64 result; // rax

  for ( i = sub_924(); *(_WORD *)i != 0xFFFF; i += result )
  {
    if ( *(_WORD *)i == 4 )
    {
      result = i;
      if ( !i || *a1 == *(_QWORD *)(i + 8) && a1[1] == *(_QWORD *)(i + 16) )
        return result;
      i = *(unsigned __int16 *)(i + 2);
    }
    else
    {
      result = *(unsigned __int16 *)(i + 2);
    }
  }
  return 0;
}

// ==================== 0x9E8: sub_9E8 ====================
__int64 sub_9E8()
{
  (*(void (__fastcall **)(void *, _QWORD, __int64 *))(qword_10F8 + 320))(&unk_1040, 0, &qword_1108);
  return (*(__int64 (__fastcall **)(void *, _QWORD, __int64 *))(qword_10F8 + 320))(&unk_1030, 0, &qword_1110);
}

// ==================== 0xA2C: sub_A2C ====================
unsigned __int64 __fastcall sub_A2C(_BYTE *a1, char a2, _QWORD *a3, __int64 a4)
{
  __int64 v4; // rax
  __int64 v9; // rbx
  __int64 v10; // rdi
  __int64 v11; // rdx
  _QWORD v12[5]; // [rsp+20h] [rbp-28h] BYREF
  int v13; // [rsp+58h] [rbp+10h] BYREF

  LOBYTE(v13) = a2;
  v4 = qword_1108;
  if ( !qword_1108 )
  {
    sub_9E8();
    v4 = qword_1108;
    if ( !qword_1108 )
      return 0x8000000000000006uLL;
  }
  v13 = 0;
  v9 = 0;
  while ( 1 )
  {
    v10 = (*(__int64 (__fastcall **)(__int64, _QWORD *, int *, __int64))(v4 + 8))(v9, v12, &v13, a4);
    if ( v10 == 0x800000000000000EuLL )
      return v10;
    v11 = v12[0];
    ++v9;
    if ( (_BYTE *)(v12[0] + 16LL) == a1 )
      goto LABEL_10;
    if ( !sub_340((_BYTE *)(v12[0] + 16LL), a1, 8) )
    {
      v11 = v12[0];
LABEL_10:
      *a3 = v11;
      return v10;
    }
    v4 = qword_1108;
  }
}

// ==================== 0xAEC: sub_AEC ====================
signed __int64 __fastcall sub_AEC(_BYTE *a1, char a2, int a3, char *a4, unsigned __int64 a5)
{
  signed __int64 result; // rax
  signed __int64 v9; // rdi
  _BYTE *v10; // rbx
  unsigned __int64 v11; // rax
  unsigned __int64 v12; // rcx
  bool i; // cc
  unsigned int v14; // eax
  _BYTE *v15; // r8
  char v16; // cl
  __int64 v17; // rdx
  __int64 v18; // [rsp+20h] [rbp-18h] BYREF
  unsigned __int64 v19; // [rsp+28h] [rbp-10h] BYREF

  if ( !qword_1110 )
  {
    sub_9E8();
    if ( !qword_1110 )
      return 0x8000000000000006uLL;
  }
  v18 = 0;
  result = sub_A2C(a1, a2, &v19, (__int64)&v18);
  v9 = result;
  if ( result >= 0 )
  {
    v10 = (_BYTE *)v19;
    if ( v19 )
    {
      v11 = v19;
      v12 = v19 + *(unsigned int *)(v19 + 4);
      for ( i = v19 <= v12; i; i = v11 <= v12 )
      {
        if ( *(_DWORD *)v11 == a3 && *(_BYTE *)(v11 - 1) == 8 )
        {
          if ( (a5 != 2 || *(_BYTE *)(v11 + 4) != 11) && (a5 != 4 || *(_BYTE *)(v11 + 4) != 12) )
            return 0x8000000000000004uLL;
          if ( (char *)(v11 + 5) != a4 )
            sub_2A0((char *)(v11 + 5), a4, a5);
          v14 = *((_DWORD *)v10 + 1);
          v15 = v10 + 9;
          v16 = 0;
          v17 = v14;
          v10[9] = 0;
          if ( v14 )
          {
            do
            {
              v16 += *v10++;
              --v17;
            }
            while ( v17 );
          }
          *v15 = -v16;
          return v9;
        }
        ++v11;
      }
    }
    return 0x800000000000000EuLL;
  }
  return result;
}

// ==================== 0xC04: sub_C04 ====================
unsigned __int64 __fastcall sub_C04(unsigned int a1)
{
  if ( (_BYTE)a1 )
    return ((unsigned __int16)(a1 & 0xFF00) << 12)
         | ((a1 & 0x1F0000 | (unsigned __int64)((a1 >> 8) & 0xE000)) >> 1)
         | ((unsigned __int64)(unsigned __int8)a1 << 32);
  else
    return ((unsigned __int16)(a1 & 0xFF00) << 12) | ((a1 & 0x1F0000 | (unsigned __int64)((a1 >> 8) & 0xE000)) >> 1);
}

// ==================== 0xC44: sub_C44 ====================
__int16 __fastcall sub_C44(unsigned int a1)
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

// ==================== 0xCB8: sub_CB8 ====================
char __fastcall sub_CB8(unsigned int a1)
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
  v2 = sub_C04(a1);
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
        sub_C44(v1);
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
          v3 = sub_C04(v6);
          if ( *(_WORD *)(((unsigned int)v3 | (v3 >> 4) & 0x10000000) + 0xC0000000) != 0xFFFF )
          {
            LOBYTE(v3) = sub_EEC(v3);
            if ( (_BYTE)v3 )
            {
              v6 = v6 & 0xFFFFFF | ((unsigned __int8)v3 << 24);
              LOBYTE(v3) = sub_CB8(v6);
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
      LOBYTE(v3) = sub_C44(v1);
  }
  return v3;
}

// ==================== 0xEEC: sub_EEC ====================
char __fastcall sub_EEC(__int64 a1)
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

