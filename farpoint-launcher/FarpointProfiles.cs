using System;
using System.IO;
using System.IO.Compression;
using System.Text;
using System.Text.RegularExpressions;
using System.Security.Cryptography;
public static class FarpointProfiles {
 static byte[] Hash(byte[] b) { using(var s=SHA1.Create()) return s.ComputeHash(b); }
 static string Str(BinaryReader r) { int n=r.ReadInt32(); if(n==0||Math.Abs((long)n)>1048576) throw new InvalidDataException("Invalid pak string"); return (n<0?Encoding.Unicode:Encoding.UTF8).GetString(r.ReadBytes(Math.Abs(n)*(n<0?2:1))).TrimEnd('\0'); }
 static void Str(BinaryWriter w,string s) { byte[] b=Encoding.UTF8.GetBytes(s+"\0");w.Write(b.Length);w.Write(b); }
 static byte[] ReadConfig(string path, string requested = "Refuge/Config/DefaultEngine.ini") {
  using(var f=File.OpenRead(path)) using(var r=new BinaryReader(f)) {
   f.Position=f.Length-44; if(r.ReadUInt32()!=0x5a6f12e1||r.ReadUInt32()!=3) throw new InvalidDataException("Requires Farpoint 1.00 pak version 3");
   long off=r.ReadInt64(),len=r.ReadInt64();byte[] hash=r.ReadBytes(20);
   if(off<0||len<0||len>64*1024*1024||off+len>f.Length-44)throw new InvalidDataException("Invalid pak index");
   f.Position=off;byte[] index=r.ReadBytes((int)len);if(BitConverter.ToString(Hash(index))!=BitConverter.ToString(hash))throw new InvalidDataException("Pak index checksum failed");
   using(var ir=new BinaryReader(new MemoryStream(index))) {
    Str(ir); uint count=ir.ReadUInt32();if(count>1000000)throw new InvalidDataException("Invalid pak count");
    for(uint i=0;i<count;i++) {
     string name=Str(ir);long start=ir.BaseStream.Position;
     long pos=ir.ReadInt64(),size=ir.ReadInt64(),unc=ir.ReadInt64();uint method=ir.ReadUInt32();ir.ReadBytes(20);
     long[] begin=new long[0],end=new long[0];if(method!=0){int n=ir.ReadInt32();if(n<0||n>1000000)throw new InvalidDataException();begin=new long[n];end=new long[n];for(int j=0;j<n;j++){begin[j]=ir.ReadInt64();end[j]=ir.ReadInt64();}}
     byte encrypted=ir.ReadByte();ir.ReadUInt32();long header=ir.BaseStream.Position-start;
     if(name!=requested)continue;
     if(encrypted!=0||unc>4*1024*1024||size>4*1024*1024)throw new InvalidDataException("Unsupported configuration entry");
     if(method==0){f.Position=pos+header;byte[] data=r.ReadBytes((int)size);if(data.Length!=size)throw new EndOfStreamException();return data;}
     if(method!=1)throw new InvalidDataException("Unsupported pak compression");
     using(var output=new MemoryStream()) {for(int j=0;j<begin.Length;j++){f.Position=begin[j];int n=checked((int)(end[j]-begin[j]));if(n<6||n>4*1024*1024)throw new InvalidDataException();byte[] b=r.ReadBytes(n);using(var z=new DeflateStream(new MemoryStream(b,2,b.Length-6),CompressionMode.Decompress))z.CopyTo(output);} if(output.Length!=unc)throw new InvalidDataException("Incorrect decompressed size");return output.ToArray();}
    }
   }
  }
  throw new InvalidDataException("Game configuration was not found");
 }
 static byte[] Entry(byte[] data) {using(var m=new MemoryStream())using(var w=new BinaryWriter(m)){w.Write((long)0);w.Write((long)data.Length);w.Write((long)data.Length);w.Write((uint)0);w.Write(Hash(data));w.Write((byte)0);w.Write((uint)0);return m.ToArray();}}
 static void WriteOverride(string target,string name,byte[] data) {
  byte[] entry=Entry(data),index;
  using(var m=new MemoryStream())using(var w=new BinaryWriter(m)){Str(w,"../../../");w.Write((uint)1);Str(w,name);w.Write(entry);index=m.ToArray();}
  using(var w=new BinaryWriter(File.Create(target+".tmp"))){w.Write(entry);w.Write(data);w.Write(index);w.Write((uint)0x5a6f12e1);w.Write((uint)3);w.Write((long)(entry.Length+data.Length));w.Write((long)index.Length);w.Write(Hash(index));}
  if(File.Exists(target))File.Delete(target);File.Move(target+".tmp",target);
 }
 public static void GenerateEnhancements(string pak,string output) {
  Directory.CreateDirectory(output);
  string configName="Refuge/Config/PS4/PS4Engine.ini";
  string config=Encoding.UTF8.GetString(ReadConfig(pak,configName));
  if(!config.Contains("r.GarlicHeapSizeMB=2400")||!config.Contains("r.NeoGarlicHeapSizeMB=2784"))throw new InvalidDataException("Unsupported memory configuration");
  config=config.Replace("r.GarlicHeapSizeMB=2400","r.GarlicHeapSizeMB=3072").Replace("r.NeoGarlicHeapSizeMB=2784","r.NeoGarlicHeapSizeMB=3072");
  config+="\n[SystemSettings]\nr.TonemapperGamma=2.0\n";
  WriteOverride(Path.Combine(output,"pakchunk101-PCVR-MemoryBudget_P.pak"),configName,Encoding.UTF8.GetBytes(config));
  string assetName="Refuge/Content/Characters/TrackedCharacter/PlayerCharacterBP.uasset";
  byte[] asset=ReadConfig(pak,assetName);
  using(var r=new BinaryReader(new MemoryStream(asset))) {
   r.BaseStream.Position=0xbd;int sprint=-1,floating=-1;
   for(int i=0;i<0x66c;i++){string name=Str(r);r.ReadUInt32();if(name=="SprintSpeedModifier")sprint=i;if(name=="FloatProperty")floating=i;}
   if(sprint<0||floating<0)throw new InvalidDataException("Unsupported sprint asset");
   byte[] tag;using(var m=new MemoryStream())using(var w=new BinaryWriter(m)){w.Write(sprint);w.Write(0);w.Write(floating);w.Write(0);w.Write(4);w.Write(0);w.Write((byte)0);tag=m.ToArray();}
   int found=-1;
   for(int i=0;i<=asset.Length-tag.Length-4;i++){bool match=true;for(int j=0;j<tag.Length;j++)if(asset[i+j]!=tag[j]){match=false;break;}if(match){if(found>=0)throw new InvalidDataException("Ambiguous sprint property");found=i+tag.Length;}}
   if(found<0||Math.Abs(BitConverter.ToSingle(asset,found)-1.35f)>0.0001f)throw new InvalidDataException("Unexpected sprint multiplier");
   Array.Copy(BitConverter.GetBytes(3.0f),0,asset,found,4);
  }
  WriteOverride(Path.Combine(output,"pakchunk100-PCVR-Sprint_P.pak"),assetName,asset);
 }
 public static void Generate(string pak,string output) {
  string original=Encoding.UTF8.GetString(ReadConfig(pak));
  if(!original.Contains("r.SceneColorFormat=2")||!original.Contains("r.BloomQuality=1")||!original.Contains("ScreenPercentage=100.000000"))throw new InvalidDataException("Unsupported game configuration");
  string baseline=original.Replace("r.SceneColorFormat=2","r.SceneColorFormat=4").Replace("r.BloomQuality=1","r.BloomQuality=0");
  int[] widths={960,1536,1920,2160,2400,2688,3072};int[] percentages={100,160,200,225,250,280,320};Directory.CreateDirectory(output);
  for(int i=0;i<widths.Length;i++) {
   string text=baseline;
   if(i>0){text=text.Replace("ScreenPercentage=100.000000","ScreenPercentage="+percentages[i]+".000000");text=text.Replace("[SystemSettings]", "[SystemSettings]\n"+"r.ScreenPercentage="+percentages[i]+"\n"+"r.NeoScreenPercentage="+percentages[i]+"\n"+"r.AutoScreenPercentage=0\n");}
   byte[] data=Encoding.UTF8.GetBytes(text),entry=Entry(data),index;
   using(var m=new MemoryStream())using(var w=new BinaryWriter(m)){Str(w,"../../../");w.Write((uint)1);Str(w,"Refuge/Config/DefaultEngine.ini");w.Write(entry);index=m.ToArray();}
   string target=Path.Combine(output,"eye-"+widths[i]+".pak");using(var w=new BinaryWriter(File.Create(target+".tmp"))){w.Write(entry);w.Write(data);w.Write(index);w.Write((uint)0x5a6f12e1);w.Write((uint)3);w.Write((long)(entry.Length+data.Length));w.Write((long)index.Length);w.Write(Hash(index));}
   if(File.Exists(target))File.Delete(target);File.Move(target+".tmp",target);
  }
 }
}
