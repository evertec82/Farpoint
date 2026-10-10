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
 static byte[] ReadConfig(string path) {
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
     if(name!="Refuge/Config/DefaultEngine.ini")continue;
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
