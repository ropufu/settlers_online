using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;

namespace Ropufu.LeytePond.Bridge
{
    class JsonCppEnumConverter<TEnum> : JsonConverter
        where TEnum : struct, IConvertible
    {
        public override Boolean CanConvert(Type objectType)
        {
            if (Object.ReferenceEquals(objectType, null)) throw new ArgumentNullException(nameof(objectType));
            return objectType.Equals(typeof(String));
        }

        public override Object ReadJson(JsonReader reader, Type objectType, Object existingValue, JsonSerializer serializer)
        {
            if (typeof(String).Equals(reader.ValueType))
            {
                var str = (String)reader.Value;
                var e = default(TEnum);
                str.CppParse(out e);
                return e;
            }
            throw new NotSupportedException();
        }

        //public override Boolean CanWrite => false;

        public override void WriteJson(JsonWriter writer, Object value, JsonSerializer serializer)
        {
            var e = (TEnum)value;
            writer.WriteValue(e.ToString().ToCpp());
        }
    }

    class JsonCppFlagsConverter<TEnum> : JsonConverter
        where TEnum : struct, IConvertible
    {
        public override Boolean CanConvert(Type objectType)
        {
            if (Object.ReferenceEquals(objectType, null)) throw new ArgumentNullException(nameof(objectType));
            return objectType.Equals(typeof(String));
        }

        public override Object ReadJson(JsonReader reader, Type objectType, Object existingValue, JsonSerializer serializer)
        {
            var strings = new List<String>();
            var flags = new List<TEnum>();

            serializer.Populate(reader, strings);

            foreach (var str in strings) flags.Add(str.CppParse<TEnum>());
            return flags;
        }

        //public override Boolean CanWrite => false;

        public override void WriteJson(JsonWriter writer, Object value, JsonSerializer serializer)
        {
            var flags = (List<TEnum>)value;
            var strings = new List<String>();
            foreach (var e in flags) strings.Add(e.ToString().ToCpp());
            serializer.Serialize(writer, strings);
        }
    }

    class JsonSkillMapConverter : JsonConverter
    {
        public override Boolean CanConvert(Type objectType)
        {
            if (Object.ReferenceEquals(objectType, null)) throw new ArgumentNullException(nameof(objectType));
            return objectType.Equals(typeof(String));
        }

        public override Object ReadJson(JsonReader reader, Type objectType, Object existingValue, JsonSerializer serializer)
        {
            if (typeof(String).Equals(reader.ValueType))
            {
                switch ((String)reader.Value)
                {
                    case "none":
                    case "empty":
                        return new SkillMap();
                }
                throw new ArgumentOutOfRangeException();
            }
            else
            {
                var json = JObject.Load(reader);
                var dictionary = new Dictionary<String, Dictionary<String, Int32>>();
                var map = new SkillMap();
                serializer.Populate(json.CreateReader(), dictionary);

                foreach (var entry in dictionary)
                {
                    if (map.Contains(entry.Key)) throw new ArgumentException();
                    var e = EnumArray<BattleSkill, Int32>.Parse(entry.Value);
                    map.Add(entry.Key, e);
                }

                return map;
            }
            throw new NotSupportedException();
        }

        //public override Boolean CanWrite => false;

        public override void WriteJson(JsonWriter writer, Object value, JsonSerializer serializer)
        {
            if (Object.ReferenceEquals(value, null)) throw new ArgumentNullException(nameof(value));
            if (!value.GetType().Equals(typeof(SkillMap))) throw new ArgumentException();

            var map = (SkillMap)value;
            var dictionary = new Dictionary<String, Dictionary<String, Int32>>();
            if (map.Count == 0) writer.WriteValue("none");
            else
            {
                var json = new JObject();
                foreach (var pair in map)
                {
                    if (pair.Value.IsEmpty) continue;
                    var arrayJson = new JObject();
                    foreach (var x in pair.Value) if (x.Value != 0) arrayJson.Add(x.Key.ToString().ToCpp(), JToken.FromObject(x.Value));
                    json.Add(pair.Key, JToken.FromObject(arrayJson, serializer));
                }
                json.WriteTo(writer);
            }
        }
    }

    /** Used when the standard converter fails. E.g., when dealing with string representation. */
    class JsonCampConverter : JsonConverter
    {
        public override Boolean CanConvert(Type objectType)
        {
            if (Object.ReferenceEquals(objectType, null)) throw new ArgumentNullException(nameof(objectType));
            return objectType.Equals(typeof(String));
        }

        public override Object ReadJson(JsonReader reader, Type objectType, Object existingValue, JsonSerializer serializer)
        {
            if (typeof(String).Equals(reader.ValueType))
            {
                switch ((String)reader.Value)
                {
                    case "none":
                    case "empty":
                        return new Camp();
                }
                throw new ArgumentOutOfRangeException();
            }
            else
            {
                var json = JObject.Load(reader);
                var camp = serializer.Deserialize<Camp>(json.CreateReader());
                return camp;
            }
            throw new NotSupportedException();
        }

        //public override Boolean CanWrite => false;

        public override void WriteJson(JsonWriter writer, Object value, JsonSerializer serializer)
        {
            if (Object.ReferenceEquals(value, null)) throw new ArgumentNullException(nameof(value));
            if (!value.GetType().Equals(typeof(Camp))) throw new ArgumentException();

            var camp = (Camp)value;
            if (camp.Equals(default(Camp))) writer.WriteValue("none");
            else
            {
                //var formatting = writer.Formatting;
                //writer.Formatting = Formatting.None;
                serializer.Serialize(writer, camp);
                //writer.Formatting = formatting;
            }
        }
    }
}
