#include "inspircd.h"

/* $ModDesc: Blocks any type of floodbot/spambot. */
/* $ModDepends: core 2.0-2.1 */
 
class ModuleAntiSpambot : public Module
{
	bool sendsnotice;
	bool msgonreply;
	LocalIntExt ext;
	std::string link;
 
 public:
	ModuleAntiSpambot()
	 : ext("waittime_timestr", this)
	{
	}
    
	
	void init()
     {
                ServerInstance->Extensions.Register(&ext);
                Implementation eventlist[] = { I_OnUserRegister, I_OnPreCommand, I_OnCheckReady, I_OnRehash };
                ServerInstance->Modules->Attach(eventlist, this, sizeof(eventlist)/sizeof(Implementation));
		OnRehash(NULL);
    }
 
	
	Version GetVersion()
	{
		return Version("Blocks any type of floodbot/spambot.", VF_VENDOR);
	}
	
	void OnRehash(User* user)
	{
		ConfigTag* tag = ServerInstance->Config->ConfValue("AntiSpambot");
		sendsnotice = tag->getBool("sendsnotice", true);
		msgonreply = tag->getBool("msgonreply", true);
		link = tag->getString("link");
	}
	
	ModResult OnUserRegister(LocalUser* user)
	{
		user->WriteNumeric(931, "%s :Malicious or potentially unwanted softwares are not WELCOME here!", user->nick.c_str());
		user->WriteServ("PRIVMSG %s :\1TIME\1", user->nick.c_str()); 
		if(sendsnotice)
                         user->WriteServ("NOTICE %s :***If you are having problems connecting to this server,please get a better CLIENT or visit: " + link + " for more infos.", user->nick.c_str());
		ext.set(user, 1); 
		return MOD_RES_PASSTHRU;
	}
	
	ModResult OnPreCommand(std::string &command, std::vector<std::string> &parameters, LocalUser* user, bool validated, const std::string &original_line)
	{
	    if (command == "NOTICE" && !validated && parameters.size() > 1 && ext.get(user))
		{
		        ext.unset(user);
			if(msgonreply)
		    user->WriteServ("NOTICE %s :***Welcome!You are authorized to use this server."user->nick.c_str());
		        return MOD_RES_DENY;
		}
		return MOD_RES_PASSTHRU;
	}
	
	ModResult OnCheckReady(LocalUser* user)
	{
		if (ext.get(user))
		return MOD_RES_DENY;
    	return MOD_RES_PASSTHRU;
	}
	
};

MODULE_INIT(ModuleAntiSpambot)
