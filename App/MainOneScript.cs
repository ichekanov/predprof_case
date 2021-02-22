using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Networking;
public class MainOneScript : MonoBehaviour
{
    string URL;
    static string[] data = new string[6];
    public Text humidity;
    public Text temperature;
    public Text motion;
    public Text lightness;
    public Text carbon_dioxyde;
    public Text sound;
    public Text DebugText;
    string Test;
    




    

    void Update()
    {
        
        for (int i = 0; i < 6; i++)
        {
           
           URL = "http://blynk-cloud.com/WToO60dj4FLXeNRw5yIBZLS3FXyQXYVi/get/V";
           string s1 = i.ToString();
           URL = URL + s1;
           StartCoroutine(Reponsing_Coroutine(i));
           data[i]= temperature.text;
                        
        }
        //temperature.text=data[0];
        //humidity.text=data[1];
        //carbon_dioxyde.text=data[2];
        //if (data[3] = "100")
        //{
        // motion.text="Detected";
        //}
        // else
        //{
        //motion.text="Not Detected";
        //}
        //lightness.text=data[4];
        //sound.text=data[5];
        //System.Threading.Thread.Sleep(500); //���� ���� �������, ����� �� ��������� ������ UPD ��� ������ ��� �����
        


        //�������������, ����������, ��� ���������. � ������ ������ ���, ��� ����. �� ������-�� ������ �� ���������. �����, � ������ ���-�� ������

    }
    private IEnumerator Reponsing_Coroutine(int i) //�������� ��� ���������� ���-��������
    {

        using (UnityWebRequest request = UnityWebRequest.Get(URL))
        {
            yield return request.SendWebRequest();
            if (request.isNetworkError) // ���� ���-�� ����� �� ���
                Test = "Error";
            else // ������� ���������� �����
            {
                switch(i)
                {
                    case 0:
                        temperature.text = request.downloadHandler.text.Trim(new char[] {'[',']','"', })+ " �";
                        break;
                    case 1:
                        humidity.text = request.downloadHandler.text.Trim(new char[] { '[', ']', '"', })+"%";
                        break;
                    case 2:
                        carbon_dioxyde.text= request.downloadHandler.text.Trim(new char[] { '[', ']', '"', })+"%";
                        break;
                    case 3:
                        motion.text = request.downloadHandler.text.Trim(new char[] { '[', ']', '"', });
                        break;
                    case 4:
                        lightness.text = request.downloadHandler.text.Trim(new char[] { '[', ']', '"', })+"%";
                        break;
                    case 5:
                        sound.text = request.downloadHandler.text.Trim(new char[] { '[', ']', '"', })+"%";
                        break;

                }
               
            }
                   
           

        }
    }
}
