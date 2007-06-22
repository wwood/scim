/** @file scim_uim_imengine.cpp
 */

/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2004 James Su <suzhe@tsinghua.org.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: scim_uim_imengine.cpp,v 1.16 2007/05/07 08:00:42 makeinu Exp $
 */

#define Uses_SCIM_UTILITY
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_CONFIG_BASE

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <scim.h>
#include "scim_uim_imengine.h"
#include <uim/uim-compat-scm.h>

#define scim_module_init uim_LTX_scim_module_init
#define scim_module_exit uim_LTX_scim_module_exit
#define scim_imengine_module_init uim_LTX_scim_imengine_module_init
#define scim_imengine_module_create_factory uim_LTX_scim_imengine_module_create_factory

#define SCIM_CONFIG_IMENGINE_UIM_UUID    "/IMEngine/UIM/UUID-"
#define SCIM_CONFIG_IMENGINE_UIM_ON_KEY  "/IMEngine/UIM/OnKey"

#define SCIM_PROP_PREFIX                 "/IMEngine/UIM"

#ifndef SCIM_UIM_ICON_FILE
    #define SCIM_UIM_ICON_FILE           (SCIM_ICONDIR "/scim-uim.png")
#endif

static const int   __uim_nr_uuids = 128;
static const char *__uim_uuids [] = {
"a7260f28-f634-49b9-bda0-9563e73dfdcc",
"0f1c9e83-87ff-4d5b-b001-d26ed9eea28b",
"feea4b74-69ca-466c-9dae-c66938277f62",
"3bf4238e-ea4e-40e2-a448-9aed0d558688",
"47903928-ca2d-4c4f-aaf3-97a32efb66f4",
"a528ac45-0576-40ce-9b56-e31a9e9dda8b",
"25ccf9a9-f532-4a0e-9ebf-c3c8d68cb736",
"2b03a434-a9bd-43e8-9c71-662931950d02",
"db4e7180-7faf-49a9-8a91-3b48b65423da",
"cf3b27aa-3c6f-4f83-8144-8a0808ebce77",
"a0a566ee-f85d-4a15-bd48-c857db390704",
"1c4660a2-516a-4ca3-bce2-8533c57bb977",
"7ce64cde-df38-4d20-97fb-053eba125ade",
"54b2928c-6e4a-4b21-8d80-19d8af37f8b2",
"527168c7-329b-44ec-bb3d-effeebc942be",
"49d4407e-08e6-447d-814a-b7b2a358b851",
"78270c67-6edd-4d71-8efb-11f262141020",
"2d4e5dde-84af-4773-b457-f84c8bf24bbd",
"c482abff-35a8-4bcb-822c-e43633126f95",
"d58644fa-8691-49f9-a06c-4969adcedabe",
"8303fcab-716d-4cb7-a83c-b529e1dfd9cc",
"50ec2062-7dfc-4948-9d3a-f853f0ee5129",
"aa53d188-4a70-4fe6-ad0f-fa785a22c33f",
"c0500ada-0f72-4dc4-9717-24e73fd7688d",
"db6edc87-7414-459f-9b38-313ffbba08c0",
"21376af6-fa66-4048-a13d-b8a4a7bc67ec",
"5b54efb7-5a37-46e8-802a-4845d9b2745f",
"00d29b16-d659-4be6-84c9-8f30ec3740ea",
"50cf1952-1ab0-4eb8-91db-5a4fc5ef20e7",
"8f8b87bf-7dbb-4de7-a50b-a78988514874",
"56b35489-1c04-4053-a922-9e29153d18f8",
"7f82f830-7311-4135-8393-5f73936f8ef1",
"c5f4ee73-76eb-4312-896b-17f5bf44dc6e",
"e6aa003b-6463-42d2-b0c5-8452a83f8b7f",
"94519d47-6b02-4f01-9662-68340aa54062",
"9bef12ea-f24a-4545-9072-44c2cb97699f",
"e7e8ccfa-1ddd-410d-a090-8143ecdacd4e",
"1b6e0b3f-9724-4e2e-b3da-489b1812d97e",
"bb3b545e-b05e-49f3-a07b-f75deb266371",
"1987ec94-fd7d-49c9-8eb5-b3b80b8fb692",
"18f1a0de-f409-4fba-974c-5aed8f61f8cf",
"088b5c7c-e57c-45aa-8a0e-a32517a58796",
"b4ed97f1-95ec-4cc1-8bb7-34b301c0f5f3",
"aba86f78-53ad-444b-9192-09d35cad3c84",
"63e2c45d-3dbe-4a31-8ae0-892e6c41a217",
"9ea81b5e-ea47-4b9b-abee-812de4bf137a",
"96a10d16-5ee4-4c22-be34-b4d4902b741d",
"1ae43bc7-cc5b-4667-a348-7ed24e78492d",
"347ac412-3db7-43f3-a6b7-e7c0f76bf350",
"6d858525-7024-440b-98a0-29916bf3b8bc",
"dff358bc-2470-4b4d-ac8a-7b2ef8ed16e8",
"cfab4949-206e-4e56-877f-9ec7e76bdfa9",
"456dfdd0-f967-4eb6-a7ef-8de0a98e9b3e",
"d462765f-4c17-4692-8597-69ef2dcc024d",
"e02d8a38-7fb7-4a11-8015-c4bbec8cffe6",
"f12a5151-b3c2-499d-ad54-ced806124859",
"4ea506b4-183b-4e54-bc0c-a9c6d810ddbf",
"51ff97b2-b41b-41a0-934c-0e6d035ad367",
"d9b2a316-9710-445f-af66-539e5dd28423",
"62ae3ad8-089c-4a74-834a-a4b8904c2d9a",
"08a98403-e50d-4f3e-a295-27464fa96c13",
"afe96cee-73c3-4af3-aed6-a1c51e5974bc",
"7c05965a-62f3-4d56-a82d-1e4b363fc5ab",
"43fd4393-8425-4bd0-9ba4-8f0d71db7659",
"6e65c275-8c23-4642-91e9-6ea9ecbda41c",
"ec7cde34-a3e3-4db6-bb36-6eea931e29d6",
"051b61df-28fd-4b7d-92ba-af140a1cf62d",
"0af548b8-7b5c-45de-8af1-444b9622bf2e",
"f4986a51-7b16-4c9a-8b90-7d16a84dba87",
"80358db4-c753-4337-a889-ea84d8135335",
"adada64e-d5a8-4bf4-b5cf-b3b22ddc8bf8",
"23c87d83-b581-4ad0-bc22-60d89d510586",
"003a0e50-6bf4-4a40-8f79-6a71d2866061",
"ef1d08d3-3d6e-4021-b191-1a5ee6a42cdd",
"ccd8cd0e-5d9a-4d7c-a066-39defcac48a8",
"224a288e-440d-4477-ae95-93dd80add396",
"bcc3267a-2e3b-46cc-8b00-fcfe1de2dee0",
"6cb55b24-0a19-4975-975e-90493eb03f20",
"853cd53c-8520-426d-ab79-06eb5830f5c9",
"1fb0a4d8-47da-4268-919c-f3ffcb0c6e55",
"a4ce45ab-7bac-4a9a-b30b-cadabf510b1e",
"1cb3271f-c661-46cb-b8b2-0e05a7f2f783",
"03c5a6ea-2b14-4758-abf0-ee4463079501",
"d44f68fd-62e9-497e-bb58-a1f1fa86a335",
"99fa1c0a-5434-4432-951f-68a9003e4524",
"6fba0759-93a6-49af-979c-ba5f542ef1b7",
"961e550f-de34-44db-97cd-11374968aa85",
"beb2cbf2-6499-4a23-8e41-8a66f98a0e52",
"18e304a2-cf55-43ab-81f4-3b5035ffe592",
"b43051d5-055b-4a5b-b4f8-2bd18396fc4e",
"51daa0f3-cd21-426d-9262-20c5f8264827",
"634ec9f1-01cb-4f7b-b350-c2713fe1b3ae",
"b39d13b8-3660-4ab3-a29e-6b839e2a2cc8",
"63310e02-2f33-4e6d-ad09-042248f1548c",
"dbcd01fe-c673-4291-bd34-aa967e93056f",
"aa47fd51-a9fc-4a22-a880-67f2ceaf4e51",
"9795ce0e-8917-4eff-a8b0-97df4923b962",
"3a941c8a-7296-4ba3-80e9-d217460fa284",
"7d5edaf0-567d-4b18-bd9f-9298eaca5863",
"3a70a0e8-9972-4b7e-9913-7fa66c858b39",
"800d4437-7e1f-4602-8428-a8166810a519",
"ffba4b45-c917-4877-a7fa-b4d598a8da3b",
"e40141ad-0fcf-4d21-ae51-da7d5ce39132",
"7f1ef850-ff54-45c0-82f3-48d329c74b25",
"ec3dafdf-01b6-4e50-af5b-1d13bcad0a07",
"90e5b710-a9be-4a51-9ee5-4ff246c7d905",
"f4aa1d77-3319-47df-9c08-6e48bc64647b",
"7e541494-c453-4ee6-b625-98b0d5918fdd",
"9e8e1898-5803-487f-9934-f2543abc501b",
"5915de02-33c0-40a3-a1fd-3a4736b1c519",
"55dbc4f5-f4ea-4b5b-8890-a7413d7731f1",
"78db4c98-324b-4a83-8fc7-1e6b4d6aef63",
"c6ab096c-cf09-4642-841b-cc986cf83f49",
"21475d28-c74f-4ea5-b323-d8e37c1c3f10",
"422a2a17-a7ea-4a88-96ee-a22fdedcf25a",
"a9069165-d399-406b-a3b6-94513d1fe5dd",
"ad0d8f2f-0268-4c4f-b542-00c3cffdabf8",
"429eecad-f904-427a-9132-7f198d3fc3e7",
"a5d2fa81-5243-4798-a980-1eac5ffdfcaf",
"5d9059aa-3daa-4a55-bae2-c7ff7787eb8a",
"09bd8211-68c6-44db-99e6-33ffef0b195c",
"3ea3a87b-835b-40f0-983d-5d296ba578a3",
"44c02613-42e8-49a4-bdf6-f16e9fc5d7b8",
"d61d2879-d3f7-4da1-aabf-c124641595e0",
"430cbebd-56bc-45d0-b0c4-8f8fbe8e1590",
"fd8f93df-0c91-49ec-9442-282dabc0d3d7",
"ff053f70-ebc0-462a-a52d-559435d5d390",
"a7027b54-ed6e-4656-80cc-3b1641853efa"
};

struct UIMInfo {
    String name;
    String lang;
    String uuid;
};

// first = name, second = lang
static std::vector <UIMInfo> __uim_input_methods;
static KeyEvent              __uim_on_key;

static ConfigPointer _scim_config (0);

extern "C" {
    void scim_module_init (void)
    {
    }

    void scim_module_exit (void)
    {
        uim_quit ();
    }

    uint32 scim_imengine_module_init (const ConfigPointer &config)
    {
        SCIM_DEBUG_IMENGINE(1) << "Initialize UIM Engine.\n";

        _scim_config = config;

        if (uim_init ()) {
            SCIM_DEBUG_IMENGINE(1) << "Failed to initialize UIM Library!\n";
            return 0;
        }

        String on_key = config->read (SCIM_CONFIG_IMENGINE_UIM_ON_KEY, String ("Shift+space"));

        if (!scim_string_to_key (__uim_on_key, on_key))
            __uim_on_key = KeyEvent (SCIM_KEY_space, SCIM_KEY_ShiftMask);

        uim_context uc = uim_create_context(NULL, "UTF-8", NULL,
                                            NULL, uim_iconv, NULL);

        if (!uc) return 0;

        int i;
        int nr = uim_get_nr_im(uc);
        int count = 0;

        UIMInfo im_info;
 
        SCIM_DEBUG_IMENGINE(1) << "Get all IM info:\n";

        // Get all im info.
        for (i = 0; i < nr; ++i) {
            const char *name = uim_get_im_name (uc, i);
            const char *lang = uim_get_im_language (uc, i);

            im_info.name = String (name);
            im_info.lang = String (lang);

            SCIM_DEBUG_IMENGINE(1) << "  " << name << "\t" << lang << "\n";

            // Do not account the "default" im.
            if (strncmp (name, "default", 7)) {
                __uim_input_methods.push_back (im_info);
                count ++;
            }

            if (count >= __uim_nr_uuids) break;
        }

        // Get all uuids.
        for (i = 0; i < count; ++i)
            __uim_input_methods [i].uuid = config->read (String (SCIM_CONFIG_IMENGINE_UIM_UUID) + __uim_input_methods [i].name,
                                                         String (""));

        // Allocate uuids for the IMs without uuid.
        for (i = 0; i < count; ++i) {
            if (!__uim_input_methods [i].uuid.length ()) {
                for (int j = 0; j < __uim_nr_uuids; ++j) {
                    int k = 0;
                    for (; k < count; ++k)
                        if (String (__uim_uuids [j]) == __uim_input_methods [k].uuid) break;

                    if (k == count) {

                        SCIM_DEBUG_IMENGINE(1) << "Set UUID: " << __uim_uuids [j] << " -> " << __uim_input_methods [i].name << "\n";

                        __uim_input_methods [i].uuid = __uim_uuids [j];
                        config->write (String (SCIM_CONFIG_IMENGINE_UIM_UUID) + __uim_input_methods [i].name,
                                       String (__uim_uuids [j]));
                        break;
                    }
                }
            }
        }

        return count;
    }

    IMEngineFactoryPointer scim_imengine_module_create_factory (uint32 engine)
    {
        if (engine >= __uim_input_methods.size ()) return 0;

        UIMFactory *factory = 0;

        try {
            factory = new UIMFactory (__uim_input_methods [engine].name,
                                      __uim_input_methods [engine].lang,
                                      __uim_input_methods [engine].uuid);
        } catch (...) {
            delete factory;
            factory = 0;
        }

        return factory;
    }
}

UIMFactory::UIMFactory (const String &name,
                        const String &lang,
                        const String &uuid)
    : m_name (name),
      m_uuid (uuid)
{
    SCIM_DEBUG_IMENGINE(1) << "Create UIM Factory :\n";
    SCIM_DEBUG_IMENGINE(1) << "  Name : " << name << "\n";
    SCIM_DEBUG_IMENGINE(1) << "  Lang : " << lang << "\n";
    SCIM_DEBUG_IMENGINE(1) << "  UUID : " << uuid << "\n";

    if (lang.length () >= 2)
        set_languages (lang);
}

UIMFactory::~UIMFactory ()
{
}

WideString
UIMFactory::get_name () const
{
    return utf8_mbstowcs (String ("uim-") + m_name);
}

WideString
UIMFactory::get_authors () const
{
    return WideString ();
}

WideString
UIMFactory::get_credits () const
{
    return WideString ();
}

WideString
UIMFactory::get_help () const
{
    return WideString ();
}

String
UIMFactory::get_uuid () const
{
    return m_uuid;
}

String
UIMFactory::get_icon_file () const
{
    return String (SCIM_UIM_ICON_FILE);
}

IMEngineInstancePointer
UIMFactory::create_instance (const String &encoding, int id)
{
    return new UIMInstance (this, m_name, encoding, id);
}


UIMInstance::UIMInstance (UIMFactory   *factory,
                          const String &uim_name,
                          const String &encoding,
                          int           id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_show_lookup_table (false)
{

    SCIM_DEBUG_IMENGINE(1) << "Create UIM Instance : " << uim_name << "\n";

    m_uc = uim_create_context (this, "UTF-8", NULL,
                               uim_name.c_str (), 
                               uim_iconv,
                               uim_commit_cb);

    if (m_uc) {
        uim_set_preedit_cb (m_uc,
                            uim_preedit_clear_cb,
                            uim_preedit_pushback_cb,
                            uim_preedit_update_cb);
        uim_set_prop_list_update_cb (m_uc,
                                     uim_prop_list_update_cb);
        uim_set_prop_label_update_cb (m_uc,
                                      uim_prop_label_update_cb);
        uim_set_candidate_selector_cb (m_uc,
                                       uim_cand_activate_cb,
                                       uim_cand_select_cb,
                                       uim_cand_shift_page_cb,
                                       uim_cand_deactive_cb);

        if (__uim_on_key.is_key_press ())
            uim_press_key (m_uc, convert_keycode (__uim_on_key.code), convert_keymask (__uim_on_key.mask));
        else
            uim_release_key (m_uc, convert_keycode (__uim_on_key.code), convert_keymask (__uim_on_key.mask));
    }
}

UIMInstance::~UIMInstance ()
{
    if (m_uc) uim_release_context (m_uc);
}

bool
UIMInstance::process_key_event (const KeyEvent& key)
{
    if (!m_uc) return false;

    SCIM_DEBUG_IMENGINE(2) << "process_key_event.\n";

    int rv;

    int keycode = convert_keycode (key.code);
    int keymask = convert_keymask (key.mask);

    if (key.is_key_press ())
        rv = uim_press_key (m_uc, keycode, keymask);
    else
        rv = uim_release_key (m_uc, keycode, keymask);

    return rv == 0;
}

void
UIMInstance::move_preedit_caret (unsigned int pos)
{
}

void
UIMInstance::select_candidate (unsigned int item)
{
    if (!m_uc || !m_lookup_table.number_of_candidates ()) return;

    SCIM_DEBUG_IMENGINE(2) << "select_candidate.\n";

    int current = m_lookup_table.get_cursor_pos_in_current_page ();
 
    if (current != item) {
        m_lookup_table.set_cursor_pos_in_current_page (item);
        uim_set_candidate_index (m_uc, m_lookup_table.get_cursor_pos ());
        update_lookup_table (m_lookup_table);
    }
}

void
UIMInstance::update_lookup_table_page_size (unsigned int page_size)
{
    SCIM_DEBUG_IMENGINE(2) << "update_lookup_table_page_size.\n";

    m_lookup_table.set_page_size (page_size);
}

void
UIMInstance::lookup_table_page_up ()
{
    if (!m_uc || !m_lookup_table.number_of_candidates () || !m_lookup_table.get_current_page_start ()) return;

    SCIM_DEBUG_IMENGINE(2) << "lookup_table_page_up.\n";

    m_lookup_table.page_up ();

    update_lookup_table (m_lookup_table);
    uim_set_candidate_index (m_uc, m_lookup_table.get_cursor_pos ());
}

void
UIMInstance::lookup_table_page_down ()
{
    if (!m_uc || !m_lookup_table.number_of_candidates () ||
        m_lookup_table.get_current_page_start () + m_lookup_table.get_current_page_size () >=
          m_lookup_table.number_of_candidates ())
        return;

    SCIM_DEBUG_IMENGINE(2) << "lookup_table_page_down.\n";

    m_lookup_table.page_down ();

    update_lookup_table (m_lookup_table);
    uim_set_candidate_index (m_uc, m_lookup_table.get_cursor_pos ());
}

void
UIMInstance::reset ()
{
    SCIM_DEBUG_IMENGINE(2) << "reset.\n";
    uim_reset_context (m_uc);
}

void
UIMInstance::focus_in ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_in.\n";

    hide_aux_string ();

    uim_prop_list_update (m_uc);
    uim_prop_label_update (m_uc);

    uim_preedit_update_cb (this);

    if (m_show_lookup_table && m_lookup_table.number_of_candidates ()) {
        update_lookup_table (m_lookup_table);
        show_lookup_table ();
    } else {
        hide_lookup_table ();
    }
}

void
UIMInstance::focus_out ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_out.\n";
}

void
UIMInstance::trigger_property (const String &property)
{
    String uim_prop = property.substr (property.find_last_of ('/') + 1);

    SCIM_DEBUG_IMENGINE(2) << "trigger_property : " << property << " - " << uim_prop << "\n";

    uim_prop_activate (m_uc, uim_prop.c_str ());
}

int
UIMInstance::convert_keycode (int key)
{
    switch (key) {
        case SCIM_KEY_BackSpace: return UKey_Backspace;
        case SCIM_KEY_Delete: return UKey_Delete;
        case SCIM_KEY_Escape: return UKey_Escape;
        case SCIM_KEY_Tab: return UKey_Tab;
        case SCIM_KEY_Return: return UKey_Return;
        case SCIM_KEY_Left: return UKey_Left;
        case SCIM_KEY_Up: return UKey_Up;
        case SCIM_KEY_Right: return UKey_Right;
        case SCIM_KEY_Down: return UKey_Down;
        case SCIM_KEY_Prior: return UKey_Prior;
        case SCIM_KEY_Next: return UKey_Next;
        case SCIM_KEY_Home: return UKey_Home;
        case SCIM_KEY_End: return UKey_End;
        case SCIM_KEY_Zenkaku_Hankaku: return UKey_Zenkaku_Hankaku;
        case SCIM_KEY_Multi_key: return UKey_Multi_key;
        case SCIM_KEY_Mode_switch: return UKey_Mode_switch;
        case SCIM_KEY_Henkan_Mode: return UKey_Henkan_Mode;
        case SCIM_KEY_Muhenkan: return UKey_Muhenkan;
        case SCIM_KEY_Shift_L: return UKey_Shift_key;
        case SCIM_KEY_Shift_R: return UKey_Shift_key;
        case SCIM_KEY_Control_L: return UKey_Control_key;
        case SCIM_KEY_Control_R: return UKey_Control_key;
        case SCIM_KEY_Alt_L: return UKey_Alt_key;
        case SCIM_KEY_Alt_R: return UKey_Alt_key;
        case SCIM_KEY_Meta_L: return UKey_Meta_key;
        case SCIM_KEY_Meta_R: return UKey_Meta_key;
        case SCIM_KEY_Super_L: return UKey_Super_key;
        case SCIM_KEY_Super_R: return UKey_Super_key;
        case SCIM_KEY_Hyper_L: return UKey_Hyper_key;
        case SCIM_KEY_Hyper_R: return UKey_Hyper_key;
    }
 
    if(key >= SCIM_KEY_F1 && key <= SCIM_KEY_F12) {
        return key - SCIM_KEY_F1 + UKey_F1;
    }
    if(key >= SCIM_KEY_KP_0 && key <= SCIM_KEY_KP_9) {
        return key - SCIM_KEY_KP_0 + UKey_0;
    }
    if (key < 256) {
        return key;
    }
    return UKey_Other;
}

int
UIMInstance::convert_keymask (int mod)
{
    int rv = 0;
    if (mod & SCIM_KEY_ShiftMask) {
        rv |= UMod_Shift;
    }
    if (mod & SCIM_KEY_ControlMask) {
        rv |= UMod_Control;
    }
    if (mod & SCIM_KEY_AltMask) {
        rv |= UMod_Alt;
    }
    if (mod & SCIM_KEY_SuperMask) {
        rv |= UMod_Super;
    }
    if (mod & SCIM_KEY_HyperMask) {
        rv |= UMod_Hyper;
    }
    return rv;
}

void
UIMInstance::uim_commit_cb (void *ptr, const char *str)
{
    UIMInstance *this_ptr = static_cast <UIMInstance *> (ptr);

    if (this_ptr && str) {

        SCIM_DEBUG_IMENGINE(2) << "uim_commit_cb : " << str << "\n";

        this_ptr->commit_string (utf8_mbstowcs (str));
    }
}

void
UIMInstance::uim_preedit_clear_cb (void *ptr)
{
    UIMInstance *this_ptr = static_cast <UIMInstance *> (ptr);
    if (this_ptr) {

        SCIM_DEBUG_IMENGINE(2) << "uim_preedit_clear_cb.\n";

        this_ptr->m_preedit_string = WideString ();
        this_ptr->m_preedit_attrs.clear ();
        this_ptr->m_preedit_caret = 0;
    }
}

void
UIMInstance::uim_preedit_pushback_cb (void *ptr, int attr, const char *str)
{
    UIMInstance *this_ptr = static_cast <UIMInstance *> (ptr);
    if (this_ptr && str) {

        SCIM_DEBUG_IMENGINE(2) << "uim_preedit_pushback_cb: " << attr << " " << str << "\n";

        WideString newstr (utf8_mbstowcs (str));

        if (!newstr.length () && !(attr & (UPreeditAttr_Cursor | UPreeditAttr_Separator)))
            return;

        Attribute newattr (this_ptr->m_preedit_string.length (), newstr.length (), SCIM_ATTR_DECORATE);

        if ((attr & UPreeditAttr_Separator) && !newstr.length ())
            this_ptr->m_preedit_string += utf8_mbstowcs ("|");

        if (attr & UPreeditAttr_Cursor)
            this_ptr->m_preedit_caret = this_ptr->m_preedit_string.length ();

        if (attr & UPreeditAttr_UnderLine)
            newattr.set_value (SCIM_ATTR_DECORATE_UNDERLINE);

        if (attr & UPreeditAttr_Reverse)
            newattr.set_value (SCIM_ATTR_DECORATE_REVERSE | newattr.get_value ());

        if (newstr.length ()) {
            this_ptr->m_preedit_string += newstr;
            this_ptr->m_preedit_attrs.push_back (newattr);
        }
    }
}

void
UIMInstance::uim_preedit_update_cb (void *ptr)
{
    UIMInstance *this_ptr = static_cast <UIMInstance *> (ptr);
    if (this_ptr) {

        SCIM_DEBUG_IMENGINE(2) << "uim_preedit_update_cb.\n";

        if (this_ptr->m_preedit_string.length ()) {
            this_ptr->show_preedit_string ();
            this_ptr->update_preedit_string (this_ptr->m_preedit_string,
                                             this_ptr->m_preedit_attrs);
            this_ptr->update_preedit_caret (this_ptr->m_preedit_caret);
        } else {
            this_ptr->hide_preedit_string ();
        }
    }
}


void
UIMInstance::uim_prop_list_update_cb (void *ptr, const char *str)
{
    UIMInstance *this_ptr = static_cast <UIMInstance *> (ptr);

    if (this_ptr) {

        SCIM_DEBUG_IMENGINE(2) << "uim_prop_list_update_cb:\n" << str << "\n";

        this_ptr->m_properties.clear ();

        std::vector <String> prop_strings;
        std::vector <String> prop_values;
        scim_split_string_list (prop_strings, String (str), '\n');
 
        int branch_count = 0;
        char buf [256];

        for (size_t i = 0; i < prop_strings.size (); ++ i) {
            if (prop_strings [i].length () < 1) continue;
            scim_split_string_list (prop_values, prop_strings [i], '\t');
            if (prop_values.size () < 4) continue;

            if (prop_values [0] == "branch") {
                ++ branch_count;

                // skip im-switch menu
                if (branch_count == 1 && uim_scm_symbol_value_bool("toolbar-show-action-based-switcher-button?"))
                    continue;

                snprintf (buf, 256, "/IMEngine/UIM/branch%d", branch_count);

                Property prop (buf, prop_values [2], String (""), prop_values [3]);
                this_ptr->m_properties.push_back (prop);

                SCIM_DEBUG_IMENGINE(3) << "SCIM Prop = " << buf << "\n";
            } else if (prop_values [0] == "leaf" && prop_values.size () >= 6) {
                if (branch_count == 1 && uim_scm_symbol_value_bool("toolbar-show-action-based-switcher-button?"))
                    continue;

                snprintf (buf, 256, "/IMEngine/UIM/branch%d/%s", branch_count, prop_values [5].c_str ());

                Property prop (buf, prop_values [3], String (""), prop_values [4]);
                this_ptr->m_properties.push_back (prop);

                SCIM_DEBUG_IMENGINE(3) << "SCIM Prop = " << buf << "\n";
            }
        }

        this_ptr->register_properties (this_ptr->m_properties);
    }
}

void
UIMInstance::uim_prop_label_update_cb (void *ptr, const char *str)
{
    UIMInstance *this_ptr = static_cast <UIMInstance *> (ptr);

    if (this_ptr) {

        SCIM_DEBUG_IMENGINE(2) << "uim_prop_label_update_cb:\n" << str << "\n";

        std::vector <String> prop_strings;
        std::vector <String> prop_values;
        scim_split_string_list (prop_strings, String (str), '\n');
 
        char buf [80];

        for (size_t i = 0; i < prop_strings.size (); ++ i) {
            if (prop_strings [i].length () < 1) continue;
            scim_split_string_list (prop_values, prop_strings [i], '\t');
            if (prop_values.size () < 2) continue;

            snprintf (buf, 80, "/IMEngine/UIM/branch%d", i+1);

            PropertyList::iterator it = std::find (this_ptr->m_properties.begin (),
                                                   this_ptr->m_properties.end (),
                                                   String (buf));

            if (it != this_ptr->m_properties.end ()) {
                it->set_label (prop_values [0]);
                it->set_tip (prop_values [1]);
                this_ptr->update_property (*it);
            }
        }
    }
}

void
UIMInstance::uim_cand_activate_cb (void *ptr, int nr, int display_limit)
{
    UIMInstance *this_ptr = static_cast <UIMInstance *> (ptr);
    if (this_ptr) {

        SCIM_DEBUG_IMENGINE(2) << "uim_cand_activate_cb : " << nr << " " << display_limit << "\n";

        this_ptr->m_lookup_table.clear ();
        this_ptr->m_lookup_table.set_page_size (display_limit);

        uim_candidate new_cand;
        const char *cand_str;

        for (int i = 0; i < nr; ++i) {
            new_cand = uim_get_candidate (this_ptr->m_uc, i, i);
            cand_str = uim_candidate_get_cand_str (new_cand);
            this_ptr->m_lookup_table.append_candidate (utf8_mbstowcs (cand_str));
            uim_candidate_free (new_cand);
        }

        this_ptr->show_lookup_table ();
        this_ptr->update_lookup_table (this_ptr->m_lookup_table);
        this_ptr->m_show_lookup_table = true;
    }
}

void
UIMInstance::uim_cand_select_cb (void *ptr, int index)
{
    UIMInstance *this_ptr = static_cast <UIMInstance *> (ptr);
    if (this_ptr) {

        SCIM_DEBUG_IMENGINE(2) << "uim_cand_select_cb : " << index << "\n";

        if (index >= 0 && index < this_ptr->m_lookup_table.number_of_candidates ()) {
            this_ptr->m_lookup_table.set_cursor_pos (index);
            this_ptr->update_lookup_table (this_ptr->m_lookup_table);
        }
    }
}

void
UIMInstance::uim_cand_shift_page_cb (void *ptr, int dir)
{
    UIMInstance *this_ptr = static_cast <UIMInstance *> (ptr);
    if (this_ptr) {

        SCIM_DEBUG_IMENGINE(2) << "uim_cand_shift_page_cb : " << dir << "\n";

        if (dir) this_ptr->lookup_table_page_down ();
        else this_ptr->lookup_table_page_up ();
    }
}

void
UIMInstance::uim_cand_deactive_cb (void *ptr)
{
    UIMInstance *this_ptr = static_cast <UIMInstance *> (ptr);
    if (this_ptr) {

        SCIM_DEBUG_IMENGINE(2) << "uim_cand_deactive_cb.\n";

        this_ptr->hide_lookup_table ();
        this_ptr->m_show_lookup_table = false;
    }
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
