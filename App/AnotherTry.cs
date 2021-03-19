//"Minds of Jaque Fresco" team
// Writing in english cause some troubles with encoding
using UnityEngine;
using System.Collections;
using System.Net;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;
using System;
using System.Text;
using UnityEngine.UI;

public class AnotherTry : MonoBehaviour
{
	private bool flag = true;
	public string message;
	private MqttClient client;
	public Text lightness;
	public Text humidity;
	public Text temperature;
	public Text motion;
	public Text carbon_dioxyde;
	public Text sound;
	public Text debug;
	public string[] data = { "Waiting...", "Waiting...", "Waiting...", "Waiting...", "Waiting...", "Waiting..." };

	void Start()
	{
		print("hello");
		print(data);
		client = new MqttClient(IPAddress.Parse("62.173.149.242"), 9124, false, null); //Creating client
		string clientId = Guid.NewGuid().ToString();//Generating unique ID for device
		while (!client.IsConnected)
		{
			client.Connect(clientId, "user_e5d5320c", "pass_92aa380f");
			print("connecting...");
		}
		print("Connected");
		client.Subscribe(new string[] { "user_e5d5320c/data" }, new byte[] { MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE });//Subscribing to our client
		client.MqttMsgPublishReceived += client_MqttMsgPublishReceived; //register to message received

	}
	void client_MqttMsgPublishReceived(object sender, MqttMsgPublishEventArgs e) //Function for recieving our messages
	{
		message = Encoding.UTF8.GetString(e.Message);
	}
    void Update()
    {
		while (flag)
		{
			flag = false;
			print(message);
			data = message.Split(',');

			if (data.Length == 6)
			{
				print(data[1]);
				temperature.text = data[0] + "C";
				humidity.text = data[1] + "%";
				carbon_dioxyde.text = data[2] + "%";
				motion.text = data[3] + "%";
				lightness.text = data[4] + "%";
				sound.text = data[5] + "%";
				debug.text = data[0] + " " + data[1] + " " + data[2] + " " + data[3] + " " + data[4] + " " + data[5];
			}
			StartCoroutine(waiter());

		}
	}
	IEnumerator waiter() // Created timer in the purpose not to update data too often 
	{
		yield return new WaitForSeconds(0.5f);
		flag = true;
	}
}
