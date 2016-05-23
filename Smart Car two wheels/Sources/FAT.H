
byte BUFFER[512];
byte BPB_SecPerClus;
byte BPB_NumFATs;

word BPB_BytesPerSec;
word BPB_RsvdSecCnt;
word BPB_RootEntCnt;
word BPB_TotSec16;
word BPB_FATSz16;

dword BPB_HiddSec;
dword BPB_Firstsector;

//********************************************************************************************
//��һ������
void ReadBlock(dword LBA)
//********************************************************************************************
{
 	Rd_Block(LBA,BUFFER);
  return;
}

//********************************************************************************************
//дһ������
void WriteBlock(dword LBA)
//********************************************************************************************
{
	Wt_Block(LBA,BUFFER);
  return;
}

//********************************************************************************************
void CopyBytes(byte *s, byte *d, word size)
//********************************************************************************************
{
	word i;
	for(i = 0; i < size; i++)
		*d++ = *s++;
}

//********************************************************************************************
byte IsEqual(void* A, void* B, byte Size)
//********************************************************************************************
{
	byte i, *a = A, *b = B;
	for(i = 0; i < Size; i++)
		if(a[i] != b[i])
			return 0;
	return 1;
}

//********************************************************************************************
void EmptyBytes(byte *data, word size)
//********************************************************************************************
{
	word i;
	for(i = 0; i < size; i++)
	{
		*data++ = 0;
	}
}
//********************************************************************************************
//дBPB���ݽṹ
void FATInit(void)
//********************************************************************************************
{
	FAT_BPB* BPB = (FAT_BPB*)BUFFER;
		//����
	BPB->BPB_BytesPerSec = 0x0200;//ÿ������512�ֽ�
	BPB->BPB_SecPerClus  = 0x40;  //ÿ����64������
	BPB->BPB_RsvdSecCnt  = 0x0006;//һ����������
	BPB->BPB_NumFATs     = 0x02; //2��FAT��
	BPB->BPB_RootEntCnt  = 0x0200;//512Ŀ¼�����
	BPB->BPB_TotSec16    = 0x0000;//��������
	BPB->BPB_FATSz16     = 0xF1;//һ��FAT����ռ��������
	BPB->BPB_HiddSec     = 0x00000000;  //��FAT��ǰ���ص�������
	
	WriteBlock(0);//д������
}
//********************************************************************************************
//��ȡBPB���ݽṹ
void ReadBPB(void)
//********************************************************************************************
{
	FAT_BPB* BPB = (FAT_BPB*)BUFFER;
	
	ReadBlock(0);
  if(BUFFER[0] != 0xEB && BUFFER[0] != 0xE9)
  {
    BPB_Firstsector = BUFFER[454];
    ReadBlock(BPB_Firstsector);
  }
	
	//��ȡ���� ��С��ģʽ������ת������
	BPB_BytesPerSec = BPB->BPB_BytesPerSec>>8|BPB->BPB_BytesPerSec<<8;
	BPB_SecPerClus  = BPB->BPB_SecPerClus;
	BPB_RsvdSecCnt  = BPB->BPB_RsvdSecCnt>>8|BPB->BPB_RsvdSecCnt<<8;
	BPB_NumFATs     = BPB->BPB_NumFATs;
	BPB_RootEntCnt  = BPB->BPB_RootEntCnt>>8|BPB->BPB_RootEntCnt<<8;
	BPB_TotSec16    = BPB->BPB_TotSec16>>8|BPB->BPB_TotSec16<<8;
	BPB_FATSz16     = BPB->BPB_FATSz16>>8|BPB->BPB_FATSz16<<8;
	BPB_HiddSec     = BPB->BPB_HiddSec>>16>>8|BPB->BPB_HiddSec>>16<<8|BPB->BPB_HiddSec<<16>>8|BPB->BPB_HiddSec<<16<<8;
}
//********************************************************************************************
//��ȡFAT����ʼ������
dword FATStartSec(void)
//********************************************************************************************
{
	return BPB_RsvdSecCnt + BPB_Firstsector;
}
//********************************************************************************************
//��ȡ��Ŀ¼��ʼ������
dword DirStartSec(void)
//********************************************************************************************
{
	return FATStartSec() + BPB_NumFATs * BPB_FATSz16;
}

//********************************************************************************************
//��ȡ��������ʼ������
dword DataStartSec(void)
//********************************************************************************************
{
	return DirStartSec() + BPB_RootEntCnt * 32 / BPB_BytesPerSec;
}

//********************************************************************************************
//Ŀ¼��ռ�õ�������
word GetDirSecCount(void)
//********************************************************************************************
{
	return BPB_RootEntCnt * 32 / 512;
}

//********************************************************************************************
//��ȡһ���صĿ�ʼ����
dword ClusConvLBA(word ClusID)
//********************************************************************************************
{
	return DataStartSec() + BPB_SecPerClus * (ClusID - 2);
}

//********************************************************************************************
//��ȡ�ļ��������ָ����
word ReadFAT(word Index,unsigned char flag)
//********************************************************************************************
{
	if(flag)ReadBlock(BPB_RsvdSecCnt + Index / 256 + BPB_Firstsector); 
	return BUFFER[Index % 256 * 2] + BUFFER[Index % 256 * 2 + 1] * 256;
}

//********************************************************************************************
//д�ļ��������ָ����
void WriteFAT(word Index, word Value)
//********************************************************************************************
{
	dword SecID;
	SecID = BPB_RsvdSecCnt + Index / 256 + BPB_Firstsector;
	ReadBlock(SecID);	
	BUFFER[Index % 256 * 2] = Value % 256;
	BUFFER[Index % 256 * 2 + 1] = Value / 256;
	WriteBlock(SecID);
}
//********************************************************************************************
//дFAT2
void WriteFAT2(word Index, word Value)
//********************************************************************************************
{
	word *RAM = (word*)BUFFER;
	dword SecID;
	SecID = BPB_RsvdSecCnt + BPB_FATSz16+ Index / 256 + BPB_Firstsector;
	ReadBlock(SecID);
  BUFFER[Index % 256 * 2] = Value % 256;
	BUFFER[Index % 256 * 2 + 1] = Value / 256;
	WriteBlock(SecID);
}
//********************************************************************************************
//��ȡ��Ŀ¼�п���ʹ�õ�һ��
word GetEmptyDIR()
//********************************************************************************************
{
	word DirSecCut, DirStart, i, j, ID = 0;
	DirSecCut = GetDirSecCount();
	DirStart = DirStartSec();
	for(i = 0; i < DirSecCut; i++)
	{
		ReadBlock(DirStart + i);
		for(j = 0; j < 16; j++)
		{
			if((BUFFER[j * 32] == 0) || (BUFFER[j * 32] == 0xE5))	return ID;
			else ID++;
		}
	}
	return ID;
}

//********************************************************************************************
//��ú��ļ�����Ӧ��Ŀ¼���Լ���Ŀ¼��λ��
byte GetFileID(byte Name[11], DIR* ID)
//********************************************************************************************
{
	word DirSecCut, DirStart, i, m,Index;
	DirSecCut = GetDirSecCount();
	DirStart = DirStartSec();
	for(i = 0; i < DirSecCut; i++)
	{
		ReadBlock(DirStart + i);
		for(m = 0; m <16; m++)
		{
			if(IsEqual(Name, &((DIR*)&BUFFER[m * 32])->FileName, 11))
			{
				*ID = *((DIR*)&BUFFER[m * 32]);
				Index = i*16+m;
				return Index; //�ҵ���Ӧ��Ŀ¼��,����Ŀ¼��λ��.
			}
		}
	}
	return 0; //û���ҵ���Ӧ��Ŀ¼��,����0.
}

//********************************************************************************************
//��ȡһ���յ�FAT��
word GetNextFAT(word FATnum)
//********************************************************************************************
{
	word FAT_Count,i;
	FAT_Count = BPB_FATSz16 * 256; //FAT�������� FAT16Ϊ2���ֽ�
	for(i = FATnum; i < FAT_Count; i++)
	{
		if(ReadFAT(i,i % 256 == 0 || i == FATnum) == 0) return i;
	}
	return 0;
}

//********************************************************************************************
//��ȡ��Ŀ¼��ָ����??????????????????
void ReadDIR(word Index, DIR* Value)
//********************************************************************************************
{
	dword DirStart = DirStartSec();
	ReadBlock(DirStart + Index / 16);
	CopyBytes(&BUFFER[(Index % 16) * 32], Value, 32);
}

//********************************************************************************************
//д��Ŀ¼��ָ����???????????????????
void WriteDIR(word Index, DIR* Value)
//********************************************************************************************
{
	dword LBA = DirStartSec() + Index / 16;
	ReadBlock(LBA);
	CopyBytes(Value, &BUFFER[(Index % 16) * 32], 32);
	WriteBlock(LBA);
}
//********************************************************************************************
//����һ�����ļ�
//˵��:FileName     �ļ������������ֻ��ߴ�д��ĸ����չ��Ҳ�����
//�ļ���С���Ȳ�������,��Ϊ0;
void CreateFile(byte *FileName, FIL *file, DIR *FileDir)
//********************************************************************************************
{                                                             
	word ClusID;
  word NextFAT;
        
	EmptyBytes(FileDir, sizeof(DIR));
  CopyBytes(FileName, &(FileDir -> FileName), 11);
        
  FileDir -> FileAttrib = 0x20;//�鵵    
	FileDir -> FilePosit.Size = 0xffffff00;
	//FileDir -> FilePosit.Size = FileDir -> FilePosit.Size>>16>>8|FileDir -> FilePosit.Size>>16<<8|FileDir -> FilePosit.Size<<16>>8|FileDir -> FilePosit.Size<<16<<8;
	ClusID = 2;
	FileDir -> FilePosit.Start = ClusID>>8|ClusID<<8;   
  
  file -> Fsize = 0;
  file -> Org_clust = ClusID;
  file -> Curr_clust = ClusID;
  file -> Curr_sect = ClusConvLBA(ClusID);
  file -> Id = 0;// GetEmptyDIR();//
  file -> Curr_clustnum = 1;
  
//WriteFAT(ClusID, 0xffff);
  //WriteFAT2(ClusID, 0xffff);
 //	WriteDIR(file -> Id, FileDir);
}
//********************************************************************************************
//ɾ���ļ�
void DelFile(byte Name[11])
{
  DIR FileDir;
  word ClusID,Index;
  Index = GetFileID(Name,&FileDir);
  FileDir.FileName.NAME[0] = 0xE5; 
  ClusID =  FileDir.FilePosit.Start>>8| FileDir.FilePosit.Start<<8;
  WriteFAT(ClusID,0x0000);
  WriteFAT2(ClusID,0x0000);
  WriteDIR(Index,&FileDir);     
}
//********************************************************************************************
//********************************************************************************************
//�����ļ������,ʹ��ͱ���һ��
void CopyFAT(void)
//********************************************************************************************
{
	word FATSz16, RsvdSecCnt, i;
	FATSz16 = BPB_FATSz16;
	RsvdSecCnt = BPB_RsvdSecCnt + BPB_Firstsector;
	for(i = 0; i < FATSz16; i++)
	{
		ReadBlock(RsvdSecCnt + i);
		WriteBlock(RsvdSecCnt + FATSz16 + i);
	}
}

//********************************************************************************************
//�����ļ�������
//˵����Write       1ʱ��д����,0Ϊ������
//      Name        �ļ���֧�����ֺʹ�д��ĸ
//      Start       ��ʼ�ֽ�
//      Length      ��������
//      Data        ��������
//���ڵ�BUG�ǣ�������һ��д������������һ���ص��ֽ�����FAT�������ʱ�����ִ���

void OperateFile(byte Write ,byte Name[11], dword Start, dword Length, byte* data, FIL *file,DIR *FileDir)
//********************************************************************************************
{
	dword BytePerClus, SecPerClus, ClusNum,ClusNext,ClusID,LBA,i,j;
	

	SecPerClus = BPB_SecPerClus;
	BytePerClus = BPB_SecPerClus * 512; //ÿ�ص��ֽ���

	//���㿪ʼλ�����ڴصĴغ�
	ClusNum = file -> Fsize / BytePerClus + 1;
	ClusID = file -> Curr_clust;
   
  //һ��ֻ������һ��
	for(i = file -> Curr_clustnum; i < ClusNum; i++)
	{
     ClusNext = file -> Curr_clust + 1;//GetNextFAT(file -> Curr_clust + 1);     
		 //WriteFAT(ClusID,ClusNext);
     //WriteFAT2(ClusID,ClusNext);
	   
	   file -> Curr_clust = ClusNext;
	   ClusID = ClusNext;
	   //WriteFAT(ClusID,0xffff);
     //WriteFAT2(ClusID,0xffff);
     file -> Curr_clustnum++;
	}
   
  //�����ļ���С
  file -> Fsize += Length; 	
	//FileDir -> FilePosit.Size = file -> Fsize>>16>>8|file -> Fsize>>16<<8|file -> Fsize<<16>>8|file -> Fsize<<16<<8;
	//WriteDIR(file -> Id,FileDir);
	
	//���㿪ʼλ��������������ƫ��
	i = (Start % BytePerClus) / 512;

	//���㿪ʼλ��������ƫ��
	j = (Start % BytePerClus) % 512;

	LBA = ClusConvLBA(ClusID) + i;
  
  WriteBlock(LBA);
  /*
	for(j=0; j < 512; j++)
	{
		if(Write) BUFFER[j] = *data++;
		else *data++ = BUFFER[j];

		//�����ȡ��ɾ��˳�
		if(--Length == 0)
		{
			if(Write) WriteBlock(LBA); //��д����
			return;
		}
	}
	*/
	
}


//********************************************************************************************
//��ʽ��
//�������������ϵĸ�ʽ����ֻ�������Ŀ¼�Լ�FAT��
void FormatCard(void)
//********************************************************************************************
{
    word i,DirStart,DirSecNum,FatStart,FatSecNum;
   
    ReadBPB();
    //�����Ŀ¼
    for(i = 0; i <= 512;i++ )BUFFER[i]=0;
    DirSecNum = GetDirSecCount();
    DirStart = DirStartSec();
    for(i=0;i<DirSecNum;i++)
    {
      WriteBlock(DirStart+i);
    }
    
    //���FAT��
    FatStart = BPB_RsvdSecCnt + BPB_Firstsector;
    FatSecNum = BPB_FATSz16;
    for(i=0;i<FatSecNum;i++)
    {
      WriteBlock(FatStart+i);
    }
    WriteFAT(0,0xf8ff);
    WriteFAT(1,0xffff);
    
    //����FAT��
    CopyFAT();
}

//********************************************************************************************
//---------------------------------------------------------------------------------------------------------------------------------
        
        