from pathlib import Path
import math, wave, struct, random
import numpy as np
from PIL import Image, ImageFilter
ROOT=Path(__file__).resolve().parents[1]
tex_root=ROOT/'Content/Blocks/Source'
tex_root.mkdir(parents=True,exist_ok=True)
audio_root=ROOT/'Content/Audio/Source'
audio_root.mkdir(parents=True,exist_ok=True)
model_root=ROOT/'Content/Models/Source'
model_root.mkdir(parents=True,exist_ok=True)

# Seamless procedural material textures (original generated artwork).
N=512
y,x=np.mgrid[0:N,0:N]
rng=np.random.default_rng(481516)
noise0=rng.random((N,N)).astype(np.float32)

def periodic_noise(scale, seed):
    rr=np.random.default_rng(seed)
    out=np.zeros((N,N),np.float32)
    for freq,amp in [(1,0.45),(2,0.24),(4,0.14),(8,0.08),(16,0.05),(32,0.03)]:
        phase1=rr.random()*2*np.pi; phase2=rr.random()*2*np.pi
        out += amp*np.sin(2*np.pi*freq*x/N+phase1)*np.cos(2*np.pi*freq*y/N+phase2)
        out += amp*0.42*np.sin(2*np.pi*freq*(x+y)/N+phase2)
    return out

def pattern(name, seed):
    base=periodic_noise(1,seed)
    fine=rng_from(seed).normal(0,0.11,(N,N)).astype(np.float32)
    if name in ('grass','dirt'):
        speck=np.sin((x*0.27 + np.sin(y*.08)*9))*np.cos(y*.19)
        h=base*.30+fine*.22+speck*.08
    elif name in ('stone','cobblestone','sandstone'):
        cracks=np.sin(x*.035 + np.sin(y*.014)*4)*np.cos(y*.039 + np.sin(x*.01)*3)
        h=base*.45+fine*.15+cracks*.12
    elif name in ('sand','gravel'):
        ripple=np.sin(y*.09 + np.sin(x*.012)*2.1)*.22
        grains=np.sin(x*.21)*np.cos(y*.17)*.06
        h=base*.28+fine*.12+ripple+grains
    elif name in ('wood','planks'):
        grain=np.sin(y*.075 + np.sin(x*.012)*2.7)*.20 + np.sin(y*.17+x*.008)*.08
        h=base*.15+fine*.07+grain
    elif name=='leaves':
        dots=(np.sin(x*.13)*np.cos(y*.11)+np.sin((x+y)*.07))*.14
        h=base*.35+fine*.2+dots
    elif name=='snow':
        h=base*.12+fine*.08
    elif name=='ice':
        h=base*.25+fine*.10+np.sin((x+y)*.035)*.1
    elif name.endswith('ore'):
        veins=np.maximum(0,np.sin(x*.06+np.sin(y*.03)*3)*np.cos(y*.08))
        h=base*.25+fine*.17+veins*.27
    elif name=='clay':
        h=base*.30+fine*.16+np.sin(x*.04+y*.02)*.08
    else:
        h=base*.3+fine*.15
    return h.astype(np.float32)

def rng_from(seed): return np.random.default_rng(seed+919)

materials={
 'grass':((74,124,51),(52,88,35)),
 'dirt':((116,78,51),(91,57,37)),
 'stone':((130,135,139),(92,98,104)),
 'sand':((207,184,126),(168,146,97)),
 'wood':((137,91,52),(92,58,32)),
 'leaves':((67,119,57),(31,73,35)),
 'coal_ore':((105,109,112),(40,43,45)),
 'iron_ore':((142,130,116),(111,83,62)),
 'cobblestone':((121,124,126),(74,78,82)),
 'planks':((165,119,70),(120,78,42)),
 'gravel':((145,140,128),(97,93,85)),
 'sandstone':((193,167,115),(145,118,77)),
 'snow':((225,235,239),(178,199,209)),
 'ice':((145,205,221),(80,151,181)),
 'clay':((160,150,139),(117,108,99)),
 'copper_ore':((137,122,105),(58,113,99)),
 'gold_ore':((134,126,100),(190,152,57)),
 'diamond_ore':((105,120,125),(65,185,197)),
}

def save_rgb(path, arr):
    Image.fromarray(np.clip(arr,0,255).astype(np.uint8),'RGB').save(path,optimize=True)

for idx,(name,(light,dark)) in enumerate(materials.items()):
    h=pattern(name,300+idx*13)
    low=np.array(dark,dtype=np.float32); high=np.array(light,dtype=np.float32)
    t=np.clip(0.52+h*1.9,0,1)
    albedo=low[None,None,:]*(1-t[:,:,None])+high[None,None,:]*t[:,:,None]
    # Texture-specific micro-details and subtle luminance variation.
    grain=rng_from(idx+600).normal(0,3.4,(N,N,1)).astype(np.float32)
    albedo += grain
    if name in ('grass','leaves'):
        green= np.maximum(0,np.sin(x*.18+np.cos(y*.09)*3))*8
        albedo[:,:,1]+=green
    if name in ('coal_ore','iron_ore','copper_ore','gold_ore','diamond_ore'):
        veins=(np.sin(x*.06+np.sin(y*.03)*3)*np.cos(y*.08)>0.89)
        ore={'coal_ore':(27,29,31),'iron_ore':(174,131,91),'copper_ore':(64,150,123),'gold_ore':(226,185,66),'diamond_ore':(88,220,226)}[name]
        albedo[veins]=ore
    if name=='wood':
        rings=np.maximum(0,np.sin(y*.065+np.sin(x*.014)*2.4))
        albedo*= (0.82+0.22*rings[:,:,None])
    albedo=np.clip(albedo,0,255)
    # Height-derived seamless tangent-space normal map.
    hh=(h+1.5*np.mean(noise0,axis=0)[None,:]*0.005)
    gy,gx=np.gradient(hh.astype(np.float32))
    strength=7.0 if name not in ('snow','ice') else 4.0
    nx=-gx*strength; ny=-gy*strength; nz=np.ones_like(nx)
    norm=np.sqrt(nx*nx+ny*ny+nz*nz)
    normal=np.stack((nx/norm,ny/norm,nz/norm),axis=-1)
    normal_rgb=(normal*0.5+0.5)*255
    # roughness maps: naturally high for stone/dirt, lower for ice.
    rough_base=0.82 if name not in ('ice',) else 0.12
    rough=np.clip(rough_base+h*.14+rng_from(idx+900).normal(0,.035,(N,N)),0.04,1.0)
    rough_rgb=np.repeat((rough*255).astype(np.uint8)[:,:,None],3,axis=2)
    asset_key=name.replace('_ore','Ore').title()
    save_rgb(tex_root/f'T_{asset_key}_BC.png',albedo)
    save_rgb(tex_root/f'T_{asset_key}_N.png',normal_rgb)
    save_rgb(tex_root/f'T_{asset_key}_R.png',rough_rgb)

# Build low-poly / voxel-styled original OBJ models.
class Obj:
    def __init__(self): self.v=[]; self.uv=[]; self.faces=[]; self.mats=[]
    def cube(self, center, size, mat):
        cx,cy,cz=center; sx,sy,sz=[q/2 for q in size]; base=len(self.v)+1
        pts=[(-sx,-sy,-sz),(sx,-sy,-sz),(sx,sy,-sz),(-sx,sy,-sz),(-sx,-sy,sz),(sx,-sy,sz),(sx,sy,sz),(-sx,sy,sz)]
        self.v += [(cx+a,cy+b,cz+c) for a,b,c in pts]
        fs=[(1,4,3,2),(5,6,7,8),(1,2,6,5),(2,3,7,6),(3,4,8,7),(4,1,5,8)]
        for face in fs: self.faces.append((mat,tuple(base+i-1 for i in face)))
    def pyramid(self, center, radius, height, mat, sides=6):
        cx,cy,cz=center; base=len(self.v)+1
        for i in range(sides):
            a=2*math.pi*i/sides; self.v.append((cx+radius*math.cos(a),cy+radius*math.sin(a),cz))
        self.v.append((cx,cy,cz+height)); apex=len(self.v)
        for i in range(sides):
            a=base+i; b=base+(i+1)%sides; self.faces.append((mat,(a,b,apex)))
    def save(self,path,mtl_name='shared.mtl'):
        mats=sorted({f[0] for f in self.faces})
        with open(path,'w') as f:
            f.write('# Original low-poly/voxel source mesh generated for this project.\nmtllib '+mtl_name+'\n')
            for v in self.v: f.write('v %.5f %.5f %.5f\n'%(v[0]*100.0,v[1]*100.0,v[2]*100.0))
            f.write('vt 0 0\nvt 1 0\nvt 1 1\nvt 0 1\n')
            current=None
            for mat,face in self.faces:
                if current!=mat: f.write('usemtl '+mat+'\n'); current=mat
                f.write('f '+' '.join(str(i) for i in face)+'\n')
        return mats

def save_model(name, builder):
    obj=Obj(); builder(obj)
    mats=obj.save(model_root/f'{name}.obj',f'{name}.mtl')
    with open(model_root/f'{name}.mtl','w') as m:
        for mat in mats:
            m.write(f'newmtl {mat}\nKd 0.7 0.7 0.7\nKa 0.1 0.1 0.1\nKs 0.04 0.04 0.04\nNs 16\n')

# Terrain props
def oak(o):
    o.cube((0,0,1.7),(.48,.48,3.4),'Bark')
    for z,r in [(2.7,1.25),(3.5,1.05),(4.15,.72)]:
        for ix in range(-2,3):
            for iy in range(-2,3):
                if ix*ix+iy*iy<=4 and random.Random(ix*71+iy*19+int(z*9)).random()>.13:
                    o.cube((ix*.48,iy*.48,z),(.58,.58,.58),'Canopy')
    o.cube((0,0,3.75),(.95,.95,.48),'CanopyLight')
save_model('SM_OakTree',oak)

def pine(o):
    o.cube((0,0,1.9),(.42,.42,3.8),'Bark')
    for z,r in [(1.2,1.25),(2.2,1.0),(3.1,.72),(3.9,.45)]:
        o.pyramid((0,0,z),r,1.7,'PineNeedles',6)
save_model('SM_PineTree',pine)

def boulder(o):
    # stepped stones for chunky stylized realism
    o.cube((0,0,.42),(1.7,1.45,.85),'Rock')
    o.cube((-.2,.05,1.0),(1.05,1.1,.62),'RockLight')
    o.cube((.48,-.25,.75),(.58,.7,.48),'Rock')
save_model('SM_Boulder',boulder)

def bush(o):
    for x1,y1,z in [(-.3,0,.35),(.2,.1,.38),(0,-.25,.55),(0,.25,.58)]: o.cube((x1,y1,z),(.62,.62,.62),'Leaves')
save_model('SM_Bush',bush)

# Friendly wildlife: simple geometric rigs with leg sections.
def animal_builder(kind, body_color, accent):
    def b(o):
        o.cube((0,0,.82),(1.35,.72,.8),body_color)
        o.cube((.63,0,1.15),(.55,.52,.58),body_color)
        o.cube((.91,-.1,1.22),(.12,.08,.10),'Eyes')
        o.cube((.91,.1,1.22),(.12,.08,.10),'Eyes')
        for xx in [-.43,.43]:
            for yy in [-.23,.23]: o.cube((xx,yy,.27),(.2,.2,.54),accent)
        if kind=='cow':
            o.cube((-.05,-.37,.88),(.56,.08,.48),'Patches')
            o.cube((-.25,.37,.78),(.46,.08,.32),'Patches')
        if kind=='chicken':
            o.cube((.75,0,1.53),(.18,.18,.25),'Comb')
            o.cube((1.02,0,1.1),(.25,.23,.15),'Beak')
        if kind=='sheep':
            for dx in [-.35,0,.35]:
                for dy in [-.22,.22]: o.cube((dx,dy,1.18),(.42,.4,.38),'Wool')
    return b
save_model('SM_Creature_Sheep',animal_builder('sheep','Wool','Hooves'))
save_model('SM_Creature_Cow',animal_builder('cow','Hide','Hooves'))
save_model('SM_Creature_Chicken',animal_builder('chicken','Feathers','Legs'))
save_model('SM_Creature_Boar',animal_builder('boar','HideDark','Hooves'))

# Tools are non-weapon utility props.
def pickaxe(o):
    o.cube((0,0,.48),(.12,.12,1.25),'Handle')
    o.cube((0,0,1.16),(.92,.13,.16),'Iron')
    o.cube((-.42,0,1.03),(.16,.13,.42),'Iron')
    o.cube((.42,0,1.03),(.16,.13,.42),'Iron')
save_model('SM_Tool_Pickaxe',pickaxe)

def shovel(o):
    o.cube((0,0,.48),(.11,.11,1.3),'Handle')
    o.pyramid((0,0,1.23),.23,.4,'Iron',4)
save_model('SM_Tool_Shovel',shovel)

# Original synthesized audio loops and one-shots, 22.05kHz mono, 16-bit.
SR=22050

def write_wav(name, seconds, fn, loop=False):
    n=int(SR*seconds); samples=np.zeros(n,dtype=np.float32)
    for i in range(n): samples[i]=fn(i/SR, i/max(1,n-1))
    # fade edges for one-shots, loop uses crossfade at ends
    fade=max(1,int(SR*.035))
    samples[:fade]*=np.linspace(0,1,fade)
    samples[-fade:]*=np.linspace(1,0,fade)
    peak=max(.001,float(np.max(np.abs(samples))))
    samples=np.clip(samples*(.82/peak),-1,1)
    with wave.open(str(audio_root/f'{name}.wav'),'wb') as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(SR)
        w.writeframes(b''.join(struct.pack('<h',int(s*32767)) for s in samples))

rng=np.random.default_rng(121)
# looping forest/wind ambience, filtered noise + gentle airy harmonic
wind_noise=rng.normal(0,1,SR*8).astype(np.float32)
# smooth lowpass via convolution
wind_noise=np.convolve(wind_noise,np.ones(120)/120,mode='same')
def wind(t,p): return .35*wind_noise[int(t*SR)] + .11*math.sin(2*math.pi*.13*t)+.04*math.sin(2*math.pi*.31*t)
write_wav('AMB_Wind_Forest',8,wind,True)
water_noise=rng.normal(0,1,SR*8).astype(np.float32)
water_noise=np.convolve(water_noise,np.ones(45)/45,mode='same')
def water(t,p): return .35*water_noise[int(t*SR)]+.12*math.sin(2*math.pi*(220+9*math.sin(t*.6))*t)
write_wav('AMB_Water_Shore',8,water,True)

def noise_hit(t,p,decay=9, tone=140):
    n=rng.normal(0,1)
    return (.58*n+.42*math.sin(2*math.pi*tone*t))*math.exp(-decay*t)
write_wav('SFX_BlockBreak_Stone',.48,lambda t,p:noise_hit(t,p,11,110))
write_wav('SFX_BlockBreak_Wood',.42,lambda t,p:noise_hit(t,p,13,180))
write_wav('SFX_BlockPlace',.22,lambda t,p:(.65*math.sin(2*math.pi*165*t)+.25*math.sin(2*math.pi*245*t))*math.exp(-18*t))
write_wav('SFX_Footstep_Grass',.22,lambda t,p:noise_hit(t,p,24,95))
write_wav('SFX_UI_Click',.12,lambda t,p:(math.sin(2*math.pi*900*t)+.3*math.sin(2*math.pi*1300*t))*math.exp(-35*t))
write_wav('SFX_Animal_Soft',.7,lambda t,p:(math.sin(2*math.pi*(190+35*math.sin(t*4))*t)*.4 + .18*math.sin(2*math.pi*390*t))*math.exp(-2.8*t))

print('Generated textures',len(list(tex_root.glob('*.png'))),'models',len(list(model_root.glob('*.obj'))),'audio',len(list(audio_root.glob('*.wav'))))
