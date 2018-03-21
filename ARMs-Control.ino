////////////////////////////////////////////// PARTE DE MOTORES
#include <mcp_can.h>
#include <SPI.h>
#include "Schneider_LMD_P84.h"
#include <stdio.h>

#define ID_MOTOR_1 0x610
#define ID_MOTOR_2 0x611

///////////////////////////////////////////////////////////////////////////// PARTE DE LA IMU
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>
#include <utility/imumaths.h>
                                                        // DATOS FIJOS DEL SISTEMAS DE POLEAS Y ACTUADORES
#define RESOLUCION 0.007  //GRADOS POR PASO
#define RADIO_POLEA 25 //mm
#define ALTURA_POLEAS 360 //mm
#define D_REF 333//mm
#define DIST 50
#define H 360
#define TOL 0.035 
#define MAX 1492

double cabeceoAnterior=0;
  double cabeceoPosterior=0;
  double alabeoAnterior=0;
  double alabeoPosterior=0;

  int cont=0;
  int pasosMotor1;
  int pasosMotor2;
  int pasosMotor3;
  int pasosMotor4;
  unsigned long t=0;
  
 const double pi=3.141592;
 double deg2rad= pi/180;
///////////////////////////////////////////////////////////////////////////////////


Adafruit_BNO055 bno = Adafruit_BNO055(55);
void displayCalStatus ()
{
  /*Cogemos los cuatro velores de calibración (0...3)
   * Cualquier sensor cuyo valor sea 0 es ignorado,
   * 3 significa calibrado.
   */
   uint8_t system, gyro, accel, mag;
   system = gyro = accel = mag = 0;
   bno.getCalibration(&system, &gyro, &accel, &mag);

   Serial.print("\t");
   if(!system)
    Serial.print("! ");

   Serial.print("Sys: ");
   Serial.print(system, DEC);
   Serial.print (" G: ");
   Serial.print (gyro, DEC);
   Serial.print (" A: ");
   Serial.print (accel, DEC);
   Serial.print (" M: ");
   Serial.println(mag, DEC);
}


int calcularPasos1D(double cabeceo,double resolucion,double radioPolea,double distCentro)
{ 
  double PASOS;
  double tangente= tan(cabeceo);
  //PASOS=(((tangente*distCentro)/(2*pi*radioPolea))*(360/resolucion)) ;
  PASOS=51200*cabeceo/(2*pi);
  
  int aux=int(PASOS);
  double aux2=abs(PASOS)-abs(aux);
    
  if(abs(aux2)>0.5)
    {
      if (aux>0)
      aux++;
      else
      aux--;
    }
   
  return aux;
  
}

int calcularPasos2D(double cabeceo,double alabeo ,double resolucion,double radioPolea,double h,double posX, double posY,double Dref)
{
    double PASOS;
  double tangenteCAB= tan(cabeceo);  //ES EL ANGULO RESPECTO EL EJE X
  double tangenteAL= tan(alabeo); //ES EL ANGULO RESPECTO EL EJE Y
  double numerador= (sqrt((tangenteCAB*h-posY)*(tangenteCAB*h-posY)+(tangenteAL*h-posX)*(tangenteAL*h-posX))-Dref);
  
  PASOS=((numerador/(2*pi*radioPolea))*(360/resolucion)) ;

  
  int aux=(int)PASOS;
  double aux2=PASOS-aux;

    if(abs(aux2)>0.5)
    {
      if (aux>0)
      aux++;
      else
      aux--;
    }
    
  return aux;
   
}
void setup(){

  ////////////////////////////////////////////////////////////////////IMU
    Serial.begin(115200);

    if(!bno.begin())
  {
    Serial.print("BNO055 no detectado");
    while(1);
  }

  delay(1000);

  bno.setExtCrystalUse(true);

  displayCalStatus();
//////////////////////////////////////////////////////////////////// CAN BUS

    while (CAN_OK != CAN.begin(CAN_1000KBPS))  {            // init can bus : baudrate = 1000k
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(500);
    }
    Serial.println("CAN BUS Shield init ok!");
    Serial.println();

    delay(200);
  
    setupMotor(ID_MOTOR_1,100000,100000,80,51200); //(long ID_motor,uint32_t Acel,uint32_t Decel, int current ,uint32_t MaxVel )
    setupMotor(ID_MOTOR_2,100000,100000,80,51200);

    long matrix[][3]={
  {0.215969994664, 0.0114713395014, 1.22666156292},
{0.213174015284, 0.0056540989317, 1.22521841526},
{0.210524126887, 0.00119585474022, 1.22429478168},
{0.208442479372, -0.00141634827014, 1.22471487522},
{0.206270903349, -0.00148646999151, 1.22573125362},
{0.202889993787, 0.000695723574609, 1.22619271278},
{0.189693659544, 0.0136091029271, 1.22912120819},
{0.18069408834, 0.0226515363902, 1.23237705231},
{0.169173002243, 0.0332304723561, 1.23587167263},
{0.14817006886, 0.0486527867615, 1.24233949184},
{0.143406912684, 0.0502708926797, 1.24484610558},
{0.149633988738, 0.0398788973689, 1.24329042435},
{0.158472210169, 0.0311567187309, 1.24276137352},
{0.166316255927, 0.0230862703174, 1.24379324913},
{0.171323731542, 0.016205156222, 1.2440340519},
{0.173647612333, 0.0107983844355, 1.24358582497},
{0.173401221633, 0.00811504386365, 1.24449598789},
{0.170891180634, 0.00894538965076, 1.24788713455},
{0.167107179761, 0.0127594806254, 1.25284361839},
{0.162150591612, 0.018629854545, 1.2579087019},
{0.156398937106, 0.0247625913471, 1.25976252556},
{0.151948347688, 0.0287393759936, 1.25787270069},
{0.149586230516, 0.0308629572392, 1.25632810593},
{0.147433161736, 0.0329056344926, 1.25799036026},
{0.122860722244, 0.0472733005881, 1.26950037479},
{0.116033509374, 0.0504385530949, 1.27274382114},
{0.117274127901, 0.0479817539454, 1.27474081516},
{0.126195386052, 0.0400417335331, 1.27261769772},
{0.137872934341, 0.0282893460244, 1.2676718235},
{0.146768882871, 0.0175598915666, 1.2651194334},
{0.152834892273, 0.0104610770941, 1.26555001736},
{0.15769572556, 0.00810140464455, 1.26590526104},
{0.162348076701, 0.00948364567012, 1.26504480839},
{0.166592299938, 0.0129928179085, 1.26505589485},
{0.167677983642, 0.0169187821448, 1.26685321331},
{0.163129508495, 0.0215297751129, 1.26922380924},
{0.154241383076, 0.0251906421036, 1.27054071426},
{0.14301495254, 0.027923181653, 1.2712777853},
{0.135301068425, 0.0293310564011, 1.27140271664},
{0.133440032601, 0.0297246165574, 1.27140164375},
{0.137896955013, 0.029109057039, 1.27127504349},
{0.14627161622, 0.027935417369, 1.2709877491},
{0.156102970243, 0.0268681813031, 1.27062761784},
{0.166830077767, 0.0261515360326, 1.26954948902},
{0.179740175605, 0.0259357094765, 1.26744568348},
{0.194902852178, 0.0262274090201, 1.26437044144},
{0.212448850274, 0.0267869587988, 1.26076972485},
{0.227931305766, 0.0271563790739, 1.25778567791},
{0.253060609102, 0.0242885444313, 1.24743282795},
{0.262509703636, 0.0221435818821, 1.24141299725},
{0.266977727413, 0.0220040380955, 1.23839950562},
{0.265749543905, 0.023068882525, 1.23664522171},
{0.261911600828, 0.0242692697793, 1.23405981064},
{0.25865060091, 0.0255653113127, 1.23048591614},
{0.257629930973, 0.0272208135575, 1.22736155987},
{0.259159475565, 0.0288120284677, 1.2256039381},
{0.263099938631, 0.0298671033233, 1.22340619564},
{0.269227594137, 0.0303269755095, 1.21998000145},
{0.278224259615, 0.0305527932942, 1.21574318409},
{0.288233488798, 0.0307416897267, 1.21284770966},
{0.298828005791, 0.0310687925667, 1.21116232872},
{0.308607280254, 0.0309751518071, 1.20965278149},
{0.318413048983, 0.0292533133179, 1.20635473728},
{0.325627416372, 0.026323389262, 1.20239567757},
{0.332076817751, 0.0223094020039, 1.1985629797},
{0.336422979832, 0.0185582693666, 1.19554114342},
{0.339338421822, 0.0148864751682, 1.19156432152},
{0.339881718159, 0.012644385919, 1.18835234642},
{0.337551444769, 0.012153913267, 1.18649446964},
{0.331172198057, 0.0138269932941, 1.18591630459},
{0.320977687836, 0.0176038686186, 1.18645036221},
{0.307557225227, 0.0233118925244, 1.18743002415},
{0.29275521636, 0.0299913808703, 1.18896722794},
{0.276693224907, 0.0369568169117, 1.19140148163},
{0.259511619806, 0.0425463244319, 1.19379782677},
{0.24180495739, 0.0449336655438, 1.19449186325},
{0.223525762558, 0.0444260798395, 1.19328427315},
{0.202654525638, 0.0415285266936, 1.19166815281},
{0.184879213572, 0.0373979732394, 1.19149255753},
{0.170517489314, 0.0322025604546, 1.19193947315},
{0.16107673943, 0.0264101661742, 1.19081163406},
{0.157069355249, 0.0206553060561, 1.18829107285},
{0.162314966321, 0.0133911790326, 1.18681287766},
{0.169421046972, 0.0132802855223, 1.18652486801},
{0.177423715591, 0.0158042572439, 1.18657517433},
{0.184142246842, 0.0203766245395, 1.18900966644},
{0.188947856426, 0.0252904240042, 1.19231700897},
{0.193084791303, 0.0300570055842, 1.194668293},
{0.197570428252, 0.0340743586421, 1.19628989697},
{0.203228130937, 0.0375390723348, 1.1987105608},
{0.210251480341, 0.0407944098115, 1.20196890831},
{0.218184426427, 0.0431832447648, 1.20413005352},
{0.227696865797, 0.0438674502075, 1.2044454813},
{0.236834034324, 0.043174315244, 1.20504713058},
{0.244323074818, 0.0418473631144, 1.2078473568},
{0.247854664922, 0.0401877649128, 1.21130180359},
{0.248718872666, 0.0372240543365, 1.21382308006},
{0.247936084867, 0.0325656682253, 1.21539437771},
{0.24695661664, 0.0267619527876, 1.21734344959},
{0.247347861528, 0.0214694328606, 1.2204297781},
{0.249683082104, 0.0172897037119, 1.22356069088},
{0.253506422043, 0.0142057519406, 1.22554504871},
{0.259932994843, 0.0120683778077, 1.22718882561},
{0.269548386335, 0.011923420243, 1.22926199436},
{0.282761156559, 0.0145630287006, 1.23240840435},
{0.297967016697, 0.0204813685268, 1.23729407787},
{0.314301967621, 0.0289647094905, 1.24350333214},
{0.332594245672, 0.0388002432883, 1.25040984154},
{0.350116282701, 0.0478352792561, 1.25696253777},
{0.367162019014, 0.0557289607823, 1.2642210722},
{0.379205286503, 0.060736104846, 1.27065873146},
{0.386449664831, 0.0592764988542, 1.27687931061},
{0.381764441729, 0.0535327754915, 1.277785182},
{0.370773673058, 0.0450276695192, 1.27764201164},
{0.332709014416, 0.0262976028025, 1.2755150795},
{0.309381961823, 0.0193118788302, 1.2768266201},
{0.284684121609, 0.0154016604647, 1.27946221828},
{0.265542477369, 0.0158198717982, 1.28317832947},
{0.250626444817, 0.0200976021588, 1.28871667385},
{0.240961045027, 0.0267259441316, 1.29492509365},
{0.235472172499, 0.0334598496556, 1.30028140545},
{0.233706206083, 0.0363659337163, 1.30322384834},
{0.234254226089, 0.0358586572111, 1.30552637577},
{0.236251503229, 0.0322807729244, 1.30716633797},
{0.237464681268, 0.0269477833062, 1.30755019188},
{0.234256207943, 0.0221227835864, 1.30658769608},
{0.223853200674, 0.0190456323326, 1.3065237999},
{0.205006673932, 0.0188556667417, 1.3095703125},
{0.160590201616, 0.0254268795252, 1.31977498531},
{0.133675903082, 0.0331781990826, 1.32817685604},
{0.131626576185, 0.0342464894056, 1.33078730106},
{0.137392312288, 0.0314048118889, 1.33131206036},
{0.13814291358, 0.0279378779233, 1.33065795898},
{0.137753501534, 0.0245787911117, 1.33102655411},
{0.139749407768, 0.0223407112062, 1.33237028122},
{0.14573648572, 0.0227576177567, 1.33267724514},
{0.154947295785, 0.0253507625312, 1.33170258999},
{0.167889907956, 0.0295762941241, 1.33192741871},
{0.182740703225, 0.0340494923294, 1.33392512798},
{0.199495390058, 0.0371954217553, 1.33476507664},
{0.217527315021, 0.0373593494296, 1.33283257484},
{0.232922881842, 0.0350109562278, 1.330545187},
{0.246418312192, 0.0312923118472, 1.32892787457},
{0.259678065777, 0.0269402284175, 1.32615530491},
{0.27133128047, 0.0236079096794, 1.32253801823},
{0.282796919346, 0.0214242842048, 1.32032167912},
{0.293186157942, 0.0207297373563, 1.32065165043},
{0.302289515734, 0.0206574779004, 1.32192146778},
{0.30658903718, 0.0200066324323, 1.32151961327},
{0.306112706661, 0.0184967759997, 1.32052624226},
{0.301263034344, 0.0159008055925, 1.32018518448},
{0.2944008708, 0.0125945406035, 1.32028746605},
{0.287227213383, 0.00920558720827, 1.31976854801},
{0.281067758799, 0.00691282935441, 1.31866383553},
{0.275344461203, 0.00609487248585, 1.31825447083},
{0.27047726512, 0.00746821984649, 1.3189637661},
{0.26662582159, 0.0107091562822, 1.31942403316},
{0.265811443329, 0.0152933672071, 1.31844592094},
{0.278575569391, 0.0279848724604, 1.3191767931},
{0.292565345764, 0.034917537123, 1.32129764557},
{0.310302138329, 0.0403747856617, 1.3216354847},
{0.327706873417, 0.0433394163847, 1.32032895088},
{0.343272268772, 0.0445681847632, 1.31860506535},
{0.352291464806, 0.044832739979, 1.31831169128},
{0.355622023344, 0.0441114865243, 1.31848704815},
{0.351355552673, 0.0413527339697, 1.31725537777},
{0.339925855398, 0.0361823141575, 1.31473505497},
{0.323407441378, 0.0299515035003, 1.3126231432},
{0.30366641283, 0.0237703323364, 1.31105518341},
{0.283346503973, 0.0181483402848, 1.30959618092},
{0.261721074581, 0.0135556813329, 1.30875325203},
{0.243340343237, 0.0109532801434, 1.3092802763},
{0.225004911423, 0.00990201067179, 1.31069982052},
{0.206371307373, 0.0102505832911, 1.31178569794},
{0.191136345267, 0.0108792474493, 1.3120650053},
{0.177155017853, 0.0118132065982, 1.3122805357},
{0.16625636816, 0.0136219225824, 1.31343591213},
{0.159374460578, 0.0166983883828, 1.31568515301},
{0.157265752554, 0.0203796569258, 1.31745803356},
{0.159995436668, 0.024819996208, 1.31774783134},
{0.165819555521, 0.029610292986, 1.31756484509},
{0.172490417957, 0.0350690931082, 1.31936752796},
{0.178035408258, 0.0392233729362, 1.32189953327},
{0.18285985291, 0.0406878627837, 1.32256793976},
{0.187776371837, 0.038722909987, 1.32092952728},
{0.19313429296, 0.0332137010992, 1.31797528267},
{0.19642752409, 0.0257323235273, 1.31478083134},
{0.196294009686, 0.0179551243782, 1.31114208698},
{0.192082971334, 0.0116399247199, 1.30729460716},
{0.186613664031, 0.00978742726147, 1.30629110336},
{0.182591602206, 0.013241966255, 1.30834639072},
{0.182579860091, 0.0205556675792, 1.31102609634},
{0.187148451805, 0.0307012554258, 1.31351625919},
{0.195819020271, 0.0396903268993, 1.316157341},
{0.209617823362, 0.0458568073809, 1.3190805912},
{0.227258875966, 0.0466005206108, 1.32003951073},
{0.27155521512, 0.0292111132294, 1.31184637547},
{0.29018214345, 0.0142027316615, 1.30455982685},
{0.303983211517, -0.00051012012409, 1.29685819149},
{0.315249860287, -0.0130262346938, 1.28940212727},
{0.324063688517, -0.0187983270735, 1.28583991528},
{0.333325952291, -0.0177235435694, 1.28642690182},
{0.346136510372, -0.0100524565205, 1.28964221478},
{0.373724490404, 0.0153768938035, 1.30022490025},
{0.385248690844, 0.0301251132041, 1.30840277672},
{0.391511052847, 0.043399348855, 1.31631946564},
{0.391029536724, 0.051293361932, 1.3202675581},
{0.383794397116, 0.0541963018477, 1.32114541531},
{0.368497282267, 0.0523354038596, 1.32101309299},
{0.347072631121, 0.0462641082704, 1.3197439909},
{0.321167528629, 0.0366434492171, 1.31671273708},
{0.295110195875, 0.0258039161563, 1.31320953369},
{0.264415889978, 0.0131068574265, 1.30940783024},
{0.238370656967, 0.00340889114887, 1.30669641495},
{0.215473473072, -0.00311061600223, 1.30504584312},
{0.201087415218, -0.00458733132109, 1.30480611324},
{0.193592593074, -0.00104587129317, 1.30650961399},
{0.193746089935, 0.00545577006415, 1.30915367603},
{0.200829416513, 0.0138830738142, 1.31165254116},
{0.212841406465, 0.0215747449547, 1.31357181072},
{0.229415893555, 0.028507001698, 1.31556761265},
{0.244094178081, 0.0315409228206, 1.31620705128},
{0.25343427062, 0.030859682709, 1.31527388096},
{0.25395861268, 0.0273705236614, 1.3138614893},
{0.245764449239, 0.0225544087589, 1.31366419792},
{0.232839748263, 0.0186041723937, 1.31448280811},
{0.219229578972, 0.01646784693, 1.31544935703},
{0.206927314401, 0.0165891647339, 1.31626439095},
{0.196515262127, 0.019619628787, 1.31824600697},
{0.188574388623, 0.0250560566783, 1.32138633728},
{0.183382153511, 0.0309009775519, 1.32493269444},
{0.180228218436, 0.0363474562764, 1.32784152031},
{0.178916618228, 0.0395343564451, 1.32888877392},
{0.179592996836, 0.0393768064678, 1.32850778103},
{0.185772895813, 0.0300485808402, 1.32695877552},
{0.188318490982, 0.022306881845, 1.32424139977},
{0.18981026113, 0.0145859783515, 1.32127702236},
{0.191290020943, 0.00822206027806, 1.31970345974},
{0.193346232176, 0.00518318451941, 1.31978714466},
{0.196212694049, 0.00565676297992, 1.32013785839},
{0.202600508928, 0.0160948392004, 1.32401645184},
{0.205557972193, 0.0232842452824, 1.32857525349},
{0.218232750893, 0.0381292365491, 1.33486998081},
{0.230068981647, 0.0426773019135, 1.33730506897},
{0.262813806534, 0.041966881603, 1.33679330349},
{0.277138411999, 0.0365202538669, 1.33224415779},
{0.285224407911, 0.029508875683, 1.32828640938},
{0.286544024944, 0.0205733012408, 1.3252850771},
{0.281743377447, 0.0117732100189, 1.32241463661},
{0.272062987089, 0.00132124789525, 1.31827926636},
{0.262182027102, -0.00818513706326, 1.3143466711},
{0.253597915173, -0.0168488286436, 1.31077539921},
{0.249175101519, -0.0212153363973, 1.30869042873},
{0.249005302787, -0.0206128973514, 1.30695009232},
{0.25404715538, -0.0147532848641, 1.30570888519},
{0.264812707901, -0.00352551112883, 1.30649137497},
{0.278796583414, 0.0109553001821, 1.30981743336},
{0.294814795256, 0.0268145985901, 1.31452906132},
{0.312937885523, 0.041228685528, 1.31881678104},
{0.329712837934, 0.0499315597117, 1.32063508034},
{0.344850242138, 0.05305057019, 1.32018041611},
{0.355904877186, 0.0502203516662, 1.31751930714},
{0.362132161856, 0.042214371264, 1.31217694283},
{0.362581402063, 0.0321612283587, 1.30631232262},
{0.357748985291, 0.0211645122617, 1.30064237118},
{0.345654308796, 0.00981916766614, 1.29490876198},
{0.326631307602, 0.00101761077531, 1.2902610302},
{0.299821019173, -0.00446595717221, 1.28779864311},
{0.270388036966, -0.005590996705, 1.28850793839},
{0.236807078123, -0.00269345892593, 1.29298329353},
{0.20634073019, 0.0029790296685, 1.29882407188},
{0.178782641888, 0.0103691453114, 1.303201437},
{0.158183395863, 0.0173579473048, 1.30475306511},
{0.141238465905, 0.0266681145877, 1.3054523468},
{0.151185452938, 0.0268970932811, 1.29985451698},
{0.15972584486, 0.0256068594754, 1.29541087151},
{0.167567431927, 0.0258769225329, 1.29421830177},
{0.174139648676, 0.0322021469474, 1.29956901073},
{0.16838273406, 0.0365581512451, 1.29995822906},
{0.15572398901, 0.0397398173809, 1.30016195774},
{0.138896584511, 0.0417449250817, 1.30238366127},
{0.121716842055, 0.0422861874104, 1.30485236645},
{0.107759125531, 0.0408554598689, 1.30497646332},
{0.101540066302, 0.0375692658126, 1.30305373669},
{0.103351920843, 0.0328290313482, 1.30191111565},
{0.113008879125, 0.0274871960282, 1.30082368851},
{0.129119873047, 0.0225349571556, 1.29713690281},
{0.14883671701, 0.0188163369894, 1.2918587923},
{0.16879683733, 0.0165339633822, 1.28957462311},
{0.188635826111, 0.0144663462415, 1.28961455822},
{0.20942749083, 0.010791733861, 1.28724014759},
{0.253347754478, -0.000691108929459, 1.27714061737},
{0.278932929039, -0.00348042137921, 1.27409505844},
{0.308654069901, -0.00112009455916, 1.27363550663},
{0.342173904181, 0.00615211576223, 1.27538573742},
{0.410061001778, 0.0279607512057, 1.28352034092},
{0.463703215122, 0.0389961563051, 1.28654611111},
{0.485139727592, 0.0376029349864, 1.2828772068},
{0.499782800674, 0.0341129861772, 1.27787196636},
{0.509125113487, 0.0309943612665, 1.27399098873},
{0.515053927898, 0.0300847999752, 1.27385282516},
{0.517217695713, 0.0328743197024, 1.27810490131},
{0.510848164558, 0.042223405093, 1.28749239445},
{0.499141305685, 0.0444918945432, 1.28945374489},
{0.479676216841, 0.0431081429124, 1.28949427605},
{0.454553604126, 0.0375480391085, 1.28747260571},
{0.426877349615, 0.0285194646567, 1.28340351582},
{0.394734919071, 0.0173903405666, 1.27842342854},
{0.358459323645, 0.00717351259664, 1.27580630779},
{0.325749218464, 0.00170969904866, 1.27746272087},
{0.296400487423, 2.54119295278e-05, 1.28057980537},
{0.274873942137, 0.00168917607516, 1.28348362446},
{0.259435892105, 0.00593208475038, 1.28648293018},
{0.251593530178, 0.0107780266553, 1.28927087784},
{0.247252032161, 0.0160478744656, 1.2918639183},
{0.245916604996, 0.0208893679082, 1.29373276234},
{0.24794344604, 0.0236905775964, 1.29381990433},
{0.260515719652, 0.022099705413, 1.2899466753},
{0.270340055227, 0.0201316140592, 1.28649497032},
{0.284731984138, 0.0181098040193, 1.28194212914},
{0.302288264036, 0.0163887254894, 1.27828335762},
{0.318883925676, 0.0154685368761, 1.27721500397},
{0.334569513798, 0.0160364583135, 1.27749979496},
{0.344079911709, 0.0168856736273, 1.27740180492},
{0.349016457796, 0.0168433021754, 1.27690505981},
{0.350094050169, 0.0156124997884, 1.27724635601},
{0.344784975052, 0.0106513937935, 1.28077435493},
{0.337810277939, 0.00694271130487, 1.28055846691},
{0.32926133275, 0.00360327167436, 1.28044319153},
{0.31950506568, 0.00152629462536, 1.28138387203},
{0.312464684248, 0.00135508924723, 1.28221857548},
{0.309855371714, 0.00303250574507, 1.28273308277},
{0.314365804195, 0.00632187118754, 1.28351199627},
{0.324967801571, 0.0111531317234, 1.28428924084},
{0.340433746576, 0.0175766609609, 1.28470647335},
{0.358086556196, 0.0248637795448, 1.28505027294},
{0.376074016094, 0.0330971963704, 1.28670346737},
{0.392323553562, 0.0418703965843, 1.29029273987},
{0.407701522112, 0.0512627400458, 1.29715812206},
{0.423440188169, 0.0594990439713, 1.30503320694},
{0.439173310995, 0.0669910088181, 1.31187784672},
{0.450653851032, 0.0722807645798, 1.31666576862},
{0.456719100475, 0.0755543634295, 1.32159888744},
{0.456135809422, 0.0763562619686, 1.32591187954},
{0.449398994446, 0.0740128010511, 1.32646489143},
{0.441594868898, 0.0691107809544, 1.32376980782},
{0.437064528465, 0.064216569066, 1.32057857513},
{0.436640650034, 0.0599919818342, 1.31752109528},
{0.439035117626, 0.0573976524174, 1.3155053854},
{0.440101653337, 0.0570054538548, 1.31522893906},
{0.44041416049, 0.0577473901212, 1.31479334831},
{0.436808258295, 0.0629923194647, 1.3142234087},
{0.429790824652, 0.0672767311335, 1.31783640385},
{0.405492097139, 0.0758919119835, 1.32160234451},
{0.397325873375, 0.0805152058601, 1.32332909107},
{0.393943995237, 0.0860239937901, 1.32716596127},
{0.393059074879, 0.0918850749731, 1.33030128479},
{0.391674995422, 0.0966188013554, 1.33199882507},
{0.384653657675, 0.105209484696, 1.33519339561},
{0.38321146369, 0.109126351774, 1.33548521996},
{0.382197886705, 0.113093219697, 1.33697891235},
{0.381214827299, 0.115901648998, 1.3396641016},
{0.379245102406, 0.116713918746, 1.34066581726},
{0.372086018324, 0.109921239316, 1.33475387096},
{0.368883192539, 0.102690175176, 1.33065795898},
{0.3664945364, 0.0936088338494, 1.3230214119},
{0.363063871861, 0.0838442221284, 1.3150229454},
{0.360659211874, 0.0755375623703, 1.31110405922},
{0.360225856304, 0.0701127797365, 1.30952620506},
{0.362725019455, 0.066638328135, 1.3066893816},
{0.378818660975, 0.0668513178825, 1.29892873764},
{0.39414063096, 0.0709294378757, 1.29926729202},
{0.442951798439, 0.0903890281916, 1.30741941929},
{0.475144594908, 0.109552577138, 1.31555342674},
{0.496079832315, 0.131164357066, 1.32778847218},
{0.507514476776, 0.155854269862, 1.34489572048},
{0.505798280239, 0.182427838445, 1.36635649204},
{0.495665550232, 0.205922752619, 1.38599002361},
{0.482054054737, 0.227959051728, 1.40342223644},
{0.471521139145, 0.245821505785, 1.41700732708},
{0.459085673094, 0.266493350267, 1.42778432369},
{0.455513060093, 0.266938894987, 1.42397713661},
{0.453044652939, 0.262318283319, 1.41652035713},
{0.450696766376, 0.252912133932, 1.40430510044},
{0.447822481394, 0.24159181118, 1.39066267014},
{0.442062973976, 0.228246182203, 1.37565004826},
{0.433323800564, 0.215128257871, 1.36190855503},
{0.424608439207, 0.203665450215, 1.35044968128},
{0.415677577257, 0.192625299096, 1.33983635902},
{0.406624466181, 0.183350726962, 1.33093440533},
{0.394557893276, 0.175758004189, 1.32309436798},
{0.377803772688, 0.169698625803, 1.31704592705},
{0.338604956865, 0.160798072815, 1.30963397026},
{0.303467869759, 0.151237562299, 1.29888164997},
{0.291965425014, 0.144931092858, 1.2928699255},
{0.290907353163, 0.129324197769, 1.2773411274},
{0.302184104919, 0.12170047313, 1.26922130585},
{0.318693548441, 0.115425616503, 1.26266384125},
{0.339235007763, 0.109265081584, 1.25659000874},
{0.358801007271, 0.104299351573, 1.25131070614},
{0.376352846622, 0.100900150836, 1.24684262276},
{0.410092711449, 0.101275742054, 1.24059855938},
{0.425364226103, 0.101798757911, 1.23558938503},
{0.436067014933, 0.102640479803, 1.23303520679},
{0.439944297075, 0.103804014623, 1.23312413692},
{0.437275081873, 0.103957220912, 1.23124241829},
{0.431097626686, 0.102733463049, 1.22577834129},
{0.422796994448, 0.0997737720609, 1.2195289135},
{0.41205021739, 0.0959996059537, 1.21538627148},
{0.400164663792, 0.0906611084938, 1.21117675304},
{0.386712312698, 0.0834812223911, 1.20585596561},
{0.352803766727, 0.0686368718743, 1.19430804253},
{0.336493104696, 0.0619322061539, 1.19056749344},
{0.325968205929, 0.056426204741, 1.18883442879},
{0.319634258747, 0.0530001074076, 1.18798148632},
{0.313958406448, 0.0502751395106, 1.18675220013},
{0.308908879757, 0.0492494106293, 1.18698012829},
{0.307843744755, 0.0491063930094, 1.18828928471},
{0.312520980835, 0.0491632148623, 1.18864345551},
{0.320317029953, 0.049406580627, 1.18739652634},
{0.329781442881, 0.0496607124805, 1.18688392639},
{0.338410615921, 0.0510141439736, 1.18792712688},
{0.349232465029, 0.0536404661834, 1.18937575817},
{0.378893435001, 0.0617180094123, 1.19145750999},
{0.39435505867, 0.0674089789391, 1.19396984577},
{0.407910645008, 0.0759478658438, 1.19958901405},
{0.418159335852, 0.0880728289485, 1.20799958706},
{0.425935536623, 0.105278372765, 1.22114026546},
{0.428052157164, 0.122721008956, 1.23582792282},
{0.411618381739, 0.160960271955, 1.26583755016},
{0.394314914942, 0.176290333271, 1.27682745457},
{0.376193314791, 0.188249826431, 1.28550505638},
{0.36324557662, 0.195738777518, 1.29080867767},
{0.356473743916, 0.200065761805, 1.29338860512},
{0.354436188936, 0.20024035871, 1.29419529438},
{0.353543549776, 0.197500512004, 1.29264676571},
{0.352356255054, 0.192975759506, 1.2900056839},
{0.350452631712, 0.187865614891, 1.28684818745},
{0.34618806839, 0.183730274439, 1.28297388554},
{0.341935962439, 0.180284008384, 1.28001928329},
{0.338468998671, 0.176439359784, 1.27668619156},
{0.3348043859, 0.171622768044, 1.27308559418},
{0.334706664085, 0.165061160922, 1.2683287859},
{0.339277118444, 0.157193630934, 1.26149952412},
{0.359386444092, 0.139469385147, 1.24521636963},
{0.374341160059, 0.132158353925, 1.24035191536},
{0.387915402651, 0.128089383245, 1.2373701334},
{0.396952807903, 0.126952663064, 1.23605799675},
{0.398566782475, 0.128425374627, 1.23619377613},
{0.391474992037, 0.13199044764, 1.23840498924},
{0.377558678389, 0.137114956975, 1.24404919147},
{0.301928371191, 0.154126822948, 1.26077854633},
{0.277765929699, 0.157223641872, 1.26273214817},
{0.263828456402, 0.158320978284, 1.26572799683},
{0.266896009445, 0.152251556516, 1.26375317574},
{0.283028095961, 0.142611205578, 1.25936579704},
{0.300098836422, 0.13019657135, 1.25529384613},
{0.317207962275, 0.11415939033, 1.24912881851},
{0.34668597579, 0.0753155499697, 1.22655689716},
{0.359023749828, 0.0576200000942, 1.21256911755},
{0.374422043562, 0.0398857742548, 1.19902145863},
{0.392790973186, 0.0243168622255, 1.18817663193},
{0.411626905203, 0.0106509337202, 1.1774712801},
{0.429429471493, -0.0011277126614, 1.16766452789},
{0.445088982582, -0.00732111791149, 1.16271841526},
{0.460645318031, -0.00806271843612, 1.16241085529},
{0.47325694561, -0.00174463842995, 1.1690454483},
{0.478515625, 0.0114363683388, 1.1840120554},
{0.476544290781, 0.0283489190042, 1.20277786255},
{0.468873560429, 0.0474580563605, 1.22231209278},
{0.458792775869, 0.0659921094775, 1.24016928673},
{0.4475954175, 0.0831897035241, 1.2558773756},
{0.435140669346, 0.0967800021172, 1.26862430573},
{0.4217633605, 0.106490671635, 1.27856945992},
{0.409261912107, 0.110934317112, 1.28344595432},
{0.402601957321, 0.110141836107, 1.28337359428},
{0.399191230536, 0.104853361845, 1.28022181988},
{0.3964433074, 0.0956514775753, 1.27554440498},
{0.39197537303, 0.0844616219401, 1.2706040144},
{0.382683873177, 0.073121547699, 1.26587879658},
{0.369247049093, 0.0625203177333, 1.26179218292},
{0.354132443666, 0.0542165637016, 1.26004743576},
{0.33683398366, 0.0481293909252, 1.26091659069},
{0.326206326485, 0.0464568547904, 1.26338326931},
{0.319589942694, 0.0477269999683, 1.26748549938},
{0.316989153624, 0.0527128316462, 1.27359461784},
{0.316961854696, 0.0617436766624, 1.28212237358},
{0.320344924927, 0.0731915459037, 1.29107058048},
{0.328234314919, 0.0852314010262, 1.29936635494},
{0.338681399822, 0.0988718122244, 1.30969130993},
{0.349011808634, 0.110556423664, 1.3190882206},
{0.359314233065, 0.120985530317, 1.32659447193},
{0.368927001953, 0.127911582589, 1.330275774},
{0.379497140646, 0.130652889609, 1.3311971426},
{0.39033806324, 0.127971634269, 1.32935845852},
{0.402319550514, 0.119606733322, 1.32370948792},
{0.413138240576, 0.10722373426, 1.31597363949},
{0.424056380987, 0.0922269076109, 1.30738258362},
{0.433065772057, 0.078263618052, 1.29964327812},
{0.439417183399, 0.0653461515903, 1.29164814949},
{0.442779660225, 0.0555855855346, 1.28586447239},
{0.440881341696, 0.0497131496668, 1.28519189358},
{0.434364944696, 0.0472817011178, 1.2886646986},
{0.426812589169, 0.0469315052032, 1.29346227646},
{0.420583069324, 0.0480561926961, 1.29948234558},
{0.416548788548, 0.050872374326, 1.30664539337},
{0.410713940859, 0.055687405169, 1.31630527973},
{0.404939234257, 0.0606237389147, 1.32460999489},
{0.402123481035, 0.0667607486248, 1.33423435688},
{0.40424361825, 0.0737783461809, 1.34409368038},
{0.405231744051, 0.0917648896575, 1.364808321},
{0.397775501013, 0.100525662303, 1.37454605103},
{0.384630858898, 0.105111055076, 1.38092577457},
{0.367805838585, 0.104657284915, 1.3827637434},
{0.34532341361, 0.0977112576365, 1.37960159779},
{0.323050796986, 0.0861303880811, 1.3741850853},
{0.297700762749, 0.0701938793063, 1.3673992157},
{0.27217066288, 0.0525305457413, 1.3596496582},
{0.245325252414, 0.032378796488, 1.35148692131},
{0.227420166135, 0.017426263541, 1.3466398716},
{0.215029343963, 0.00484567927197, 1.3437037468},
{0.208920031786, -0.00277638924308, 1.34204685688},
{0.208291456103, -0.00588431209326, 1.34073328972},
{0.215419501066, -0.00375293218531, 1.34036338329},
{0.230850815773, 0.00333972508088, 1.33983910084},
{0.253999203444, 0.015058549121, 1.33997929096},
{0.279988110065, 0.0291394907981, 1.3418623209},
{0.306338131428, 0.0433561392128, 1.34421598911},
{0.330971121788, 0.0571773350239, 1.34760308266},
{0.353571981192, 0.0697947740555, 1.35174453259},
{0.37196958065, 0.0785284638405, 1.35387206078},
{0.384753465652, 0.0810672566295, 1.35171985626},
{0.392719835043, 0.078337110579, 1.34760355949},
{0.396826833487, 0.0706753879786, 1.34260082245},
{0.396400034428, 0.0605704374611, 1.3357077837},
{0.394058406353, 0.048872821033, 1.32584500313},


};


    
  for(int i=0;i<MAX;i++)
  {
  cabeceoPosterior=matrix[i][0]; //No estoy demasiado seguro de que sea el eje correcto
  alabeoPosterior=matrix[i][1];
  moverMotores(); 

  delay(10);
  }
}


void imprimirDatos(sensors_event_t event){

  Serial.print ("X: ");
  Serial.print (event.orientation.x,4);
  Serial.print ("\tY: ");
  Serial.print (event.orientation.y,4);
  Serial.print ("\tZ: ");
  Serial.println (event.orientation.z,4);


  Serial.print ("Pasos motor 1: ");
  Serial.print (pasosMotor1);
  Serial.print ("        Pasos motor 2: ");
  Serial.print (pasosMotor2);
  Serial.print ("      Pasos motor 3: ");
  Serial.print (pasosMotor3);
  Serial.print ("    Pasos motor 4: ");
  Serial.print (pasosMotor4);
  Serial.println(" ");
  //delay (2);
  
}


void moverMotores() {


  
  if(abs(cabeceoPosterior-cabeceoAnterior)>TOL || abs(alabeoPosterior-alabeoAnterior)>TOL ) //Esta sentencia se puede omitir
  {
      
      pasosMotor1=calcularPasos2D(cabeceoPosterior-cabeceoAnterior,alabeoPosterior-alabeoAnterior,RESOLUCION,RADIO_POLEA,H,333,0,D_REF);
      pasosMotor2=calcularPasos2D(cabeceoPosterior-cabeceoAnterior,alabeoPosterior-alabeoAnterior,RESOLUCION,RADIO_POLEA,H,0,333,D_REF);
      pasosMotor3=calcularPasos2D(cabeceoPosterior-cabeceoAnterior,alabeoPosterior-alabeoAnterior,RESOLUCION,RADIO_POLEA,H,-333,0,D_REF);
      pasosMotor4=calcularPasos2D(cabeceoPosterior-cabeceoAnterior,alabeoPosterior-alabeoAnterior,RESOLUCION,RADIO_POLEA,H,0,-333,D_REF);
      
      //pasosMotor1=calcularPasos1D(cabeceoPosterior-cabeceoAnterior,RESOLUCION,RADIO_POLEA,H);
      //pasosMotor3=calcularPasos1D(alabeoPosterior-alabeoAnterior,RESOLUCION,RADIO_POLEA,H);

      //AQUI iría la accion de movimiento
      mover(pasosMotor1,ID_MOTOR_1);//una vuelta
      mover(pasosMotor3,ID_MOTOR_2);
  
      cabeceoAnterior=cabeceoPosterior;
      alabeoAnterior=alabeoPosterior;
  }

      
  
}

void loop(){
      Serial.print("Micros: ");
      Serial.println(micros()-t);
  sensors_event_t event;
  bno.getEvent (&event);

  //imprimirDatos(event);

   
  
  t=micros();
}


