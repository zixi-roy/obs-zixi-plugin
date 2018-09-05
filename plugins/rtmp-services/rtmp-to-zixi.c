#include "rtmp-to-zixi.h"


unsigned int ZIXI_LATENCIES[] = { 100, 200, 300, 500, 1000, 1500, 2000, 2500, 3000, 4000, 5000, 6000, 8000, 10000, 12000, 14000, 16000 };
const char * ZIXI_LATENCIES_STR[] = { "100 ms", "200 ms", "300 ms", "500 ms", "1000 ms", "1500 ms", "2000 ms", "2500 ms", "3000 ms", "4000 ms", "5000 ms", "6000 ms", "8000 ms", "10000 ms", "12000 ms", "14000 ms", "16000 ms" };
unsigned int ZIXI_LATENCY_COUNT = 17;

void zixi_fill_defaults(  obs_data_t* settings){
	obs_data_set_string(settings, ZIXI_SERVICE_PROP_URL, "zixi://BROADCASTER_HOST:PORT/STREAM");
	obs_data_set_int(settings, ZIXI_SERVICE_PROP_LATENCY_ID, ZIXI_DEFAULT_LATENCY_ID);
	obs_data_set_int(settings, ZIXI_SERVICE_PROP_ENCRYPTION_TYPE, 0);
}

static bool zixi_forward_modified(obs_properties_t *ppts, obs_property_t *p,
	obs_data_t *settings)
{
	bool visible = obs_data_get_bool(settings, "zixi_fwd");
	p = obs_properties_get(ppts, ZIXI_SERVICE_PROP_URL);
	obs_property_set_visible(p, visible);
	p = obs_properties_get(ppts, ZIXI_SERVICE_PROP_PASSWORD);
	obs_property_set_visible(p, visible);
	p = obs_properties_get(ppts, ZIXI_SERVICE_PROP_LATENCY_ID);
	obs_property_set_visible(p, visible);
	p = obs_properties_get(ppts, ZIXI_SERVICE_PROP_ENCRYPTION_TYPE);
	obs_property_set_visible(p, visible);
	bool show_key = visible && obs_data_get_int(settings,ZIXI_SERVICE_PROP_ENCRYPTION_TYPE) != 0;
	p = obs_properties_get(ppts, ZIXI_SERVICE_PROP_ENCRYPTION_KEY);
	obs_property_set_visible(p, show_key);
	p = obs_properties_get(ppts, ZIXI_SERVICE_PROP_USE_ENCODER_FEEDBACK);
	obs_property_set_visible(p, visible);
	p = obs_properties_get(ppts, ZIXI_SERVICE_PROP_ENABLE_BONDING);
	obs_property_set_visible(p, visible);

	return true;
}

static bool encryption_type_changed(obs_properties_t *ppts,
	obs_property_t *p, obs_data_t *settings){
	static const char * ENCRYPTION_STR[] = {  "AES 128", "AES 192", "AES 256", "CHACHA-20","None" };
	static unsigned int ENCRYPTION_COUNT = 5;

	long long t = obs_data_get_int(settings, ZIXI_SERVICE_PROP_ENCRYPTION_TYPE);
	int show_key =  t != 4;
	obs_property_list_clear(p);
	for (unsigned int i = 0; i < ENCRYPTION_COUNT; i++) {
		obs_property_list_add_int(p, ENCRYPTION_STR[i], i);
	}

	p = obs_properties_get(ppts, ZIXI_SERVICE_PROP_ENCRYPTION_KEY);
	obs_property_set_visible(p, show_key);
	return true;
}


void add_zixi_fwd_properties(obs_properties_t *ppts){
	obs_property_t * p = obs_properties_add_bool(ppts, "zixi_fwd", obs_module_text("ZixiFwd"));
	obs_property_set_modified_callback(p, zixi_forward_modified);
	obs_properties_add_text(ppts, ZIXI_SERVICE_PROP_URL, obs_module_text("ZixiUrl"), OBS_TEXT_DEFAULT);
	obs_properties_add_text(ppts, ZIXI_SERVICE_PROP_PASSWORD, obs_module_text("ZixiPassword"), OBS_TEXT_PASSWORD);
	p = obs_properties_add_list(ppts, ZIXI_SERVICE_PROP_LATENCY_ID, obs_module_text("Latency(ms)"), OBS_COMBO_TYPE_LIST,OBS_COMBO_FORMAT_INT);
	obs_property_list_clear(p);
	for (unsigned int i = 0; i < ZIXI_LATENCY_COUNT; i++) {
		obs_property_list_add_int(p, ZIXI_LATENCIES_STR[i], i);
	}

	p = obs_properties_add_list(ppts, ZIXI_SERVICE_PROP_ENCRYPTION_TYPE, obs_module_text("EncryptionType"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_modified_callback(p, encryption_type_changed);
	p = obs_properties_add_text(ppts, ZIXI_SERVICE_PROP_ENCRYPTION_KEY, obs_module_text("EncryptionKey"),
		OBS_TEXT_PASSWORD);
	
	p = obs_properties_add_bool(ppts, ZIXI_SERVICE_PROP_USE_ENCODER_FEEDBACK, obs_module_text("UseEncoderFeedback"));
	p = obs_properties_add_bool(ppts, ZIXI_SERVICE_PROP_ENABLE_BONDING, obs_module_text("EnableBonding"));
	
	p = obs_properties_add_text(ppts, ZIXI_SERVICE_PROP_AUTO_RTMP_CHANNEL, obs_module_text("ZixiUrlFwd"), OBS_TEXT_DEFAULT);
	obs_property_set_visible(p, false);
	p = obs_properties_add_text(ppts, ZIXI_SERVICE_PROP_AUTO_RTMP_PASSWORD, obs_module_text("ZixiUrlFwd"), OBS_TEXT_DEFAULT);
	obs_property_set_visible(p, false);
	p = obs_properties_add_text(ppts, ZIXI_SERVICE_PROP_AUTO_RTMP_URL, obs_module_text("ZixiUrlFwd"), OBS_TEXT_DEFAULT);
	obs_property_set_visible(p, false);
	p = obs_properties_add_text(ppts, ZIXI_SERVICE_PROP_AUTO_RTMP_USERNAME, obs_module_text("ZixiUrlFwd"), OBS_TEXT_DEFAULT);
	obs_property_set_visible(p, false);
}

void update_zixi_settings(struct zixi_settings* zixi,  obs_data_t* settings, const char* rtmp_url, const char* rtmp_key, const char * rtmp_user, const char* rtmp_password){
	zixi->auto_rtmp_out = true;
	obs_data_set_bool(settings, ZIXI_SERVICE_PROP_USE_AUTO_RTMP_OUT, true);
	zixi->rtmp_channel = bstrdup(rtmp_key);
	zixi->rtmp_password = bstrdup(rtmp_password);
	zixi->rtmp_url = bstrdup(rtmp_url);
	zixi->rtmp_username = bstrdup(rtmp_user);
	obs_data_set_string(settings, ZIXI_SERVICE_PROP_AUTO_RTMP_CHANNEL, zixi->rtmp_channel);
	obs_data_set_string(settings, ZIXI_SERVICE_PROP_AUTO_RTMP_PASSWORD, zixi->rtmp_password);
	obs_data_set_string(settings, ZIXI_SERVICE_PROP_AUTO_RTMP_URL, zixi->rtmp_url);
	obs_data_set_string(settings, ZIXI_SERVICE_PROP_AUTO_RTMP_USERNAME, zixi->rtmp_username);
	zixi->url = bstrdup(obs_data_get_string(settings, ZIXI_SERVICE_PROP_URL)); 
	zixi->enable_bonding = obs_data_get_bool(settings, ZIXI_SERVICE_PROP_ENABLE_BONDING);
	zixi->encryption_id = obs_data_get_int(settings, ZIXI_SERVICE_PROP_ENCRYPTION_TYPE);
	if (zixi->encryption_id != 0 && zixi->encryption_id != 4) {
		zixi->encryption_key = bstrdup(obs_data_get_string(settings,ZIXI_SERVICE_PROP_ENCRYPTION_KEY));
	} else {
		zixi->encryption_key = NULL;
	}

	zixi->latency_id = obs_data_get_int(settings, ZIXI_SERVICE_PROP_LATENCY_ID);
	zixi->encoder_feedback = obs_data_get_bool(settings, ZIXI_SERVICE_PROP_USE_ENCODER_FEEDBACK);
}

void zixi_clear_settings(struct zixi_settings* settings) {
	if (settings->url) {
		bfree(settings->url);
		settings = NULL;
	}
	if (settings->encryption_key) {
		bfree(settings->encryption_key);
		settings->encryption_key = NULL;
	}
	if (settings->rtmp_channel){
		bfree(settings->rtmp_channel);
		settings->rtmp_channel = NULL;
	}
	if (settings->rtmp_password){
		bfree(settings->rtmp_password);
		settings->rtmp_password = NULL;
	}
	if (settings->rtmp_url){
		bfree(settings->rtmp_url);
		settings->rtmp_url = NULL;
	}
	if (settings->rtmp_username){
		bfree(settings->rtmp_username);
		settings->rtmp_username = NULL;
	}
}
