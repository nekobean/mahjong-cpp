class Tile:
    Null = -1
    Manzu1 = 0  # Manzu1 (一萬)
    Manzu2 = 1  # Manzu2 (二萬)
    Manzu3 = 2  # Manzu3 (三萬)
    Manzu4 = 3  # Manzu4 (四萬)
    Manzu5 = 4  # Manzu5 (五萬)
    Manzu6 = 5  # Manzu6 (六萬)
    Manzu7 = 6  # Manzu7 (七萬)
    Manzu8 = 7  # Manzu8 (八萬)
    Manzu9 = 8  # Manzu9 (九萬)
    Pinzu1 = 9  # Pinzu1 (一筒)
    Pinzu2 = 10  # Pinzu2 (二筒)
    Pinzu3 = 11  # Pinzu3 (三筒)
    Pinzu4 = 12  # Pinzu4 (四筒)
    Pinzu5 = 13  # Pinzu5 (五筒)
    Pinzu6 = 14  # Pinzu6 (六筒)
    Pinzu7 = 15  # Pinzu7 (七筒)
    Pinzu8 = 16  # Pinzu8 (八筒)
    Pinzu9 = 17  # Pinzu9 (九筒)
    Souzu1 = 18  # Souzu1 (一索)
    Souzu2 = 19  # Souzu2 (二索)
    Souzu3 = 20  # Souzu3 (三索)
    Souzu4 = 21  # Souzu4 (四索)
    Souzu5 = 22  # Souzu5 (五索)
    Souzu6 = 23  # Souzu6 (六索)
    Souzu7 = 24  # Souzu7 (七索)
    Souzu8 = 25  # Souzu8 (八索)
    Souzu9 = 26  # Souzu9 (九索)
    East = 27  # East (東)
    South = 28  # South (南)
    West = 29  # West (西)
    North = 30  # North (北)
    White = 31  # White (白)
    Green = 32  # Green (発)
    Red = 33  # Red (中)
    RedManzu5 = 34  # Red Manzu5 (赤五萬)
    RedPinzu5 = 35  # Red Pinzu5 (赤五筒)
    RedSouzu5 = 36  # Red Souzu5 (赤五索)

    Name = {
        Manzu1: "1m",
        Manzu2: "2m",
        Manzu3: "3m",
        Manzu4: "4m",
        Manzu5: "5m",
        Manzu6: "6m",
        Manzu7: "7m",
        Manzu8: "8m",
        Manzu9: "9m",
        Pinzu1: "1p",
        Pinzu2: "2p",
        Pinzu3: "3p",
        Pinzu4: "4p",
        Pinzu5: "5p",
        Pinzu6: "6p",
        Pinzu7: "7p",
        Pinzu8: "8p",
        Pinzu9: "9p",
        Souzu1: "1s",
        Souzu2: "2s",
        Souzu3: "3s",
        Souzu4: "4s",
        Souzu5: "5s",
        Souzu6: "6s",
        Souzu7: "7s",
        Souzu8: "8s",
        Souzu9: "9s",
        East: "1z",
        South: "2z",
        West: "3z",
        North: "4z",
        White: "5z",
        Green: "6z",
        Red: "7z",
        RedManzu5: "0m",
        RedPinzu5: "0p",
        RedSouzu5: "0s",
    }


class MeldType:
    Null = -1
    Pong = 0  # Pong (ポン)
    Chow = 1  # Chow (チー)
    ClosedKong = 2  # Closed kong (暗槓)
    OpenKong = 3  # Open kong (明槓)
    AddedKong = 4  # Added kong (加槓)

    Name = {
        Pong: "ポン",
        Chow: "チー",
        ClosedKong: "暗槓",
        OpenKong: "明槓",
        AddedKong: "加槓",
    }


class WaitType:
    Null = -1
    DoubleEdgeWait = 0  # waiting for either of two gates (両面待ち)
    EdgeWait = 1  # waiting for three or seven the tile (辺張待ち)
    ClosedWait = 2  # waiting for the middle in a sequence (嵌張待ち)
    TripletWait = 3  # Waiting for a triplet (双ポン待ち)
    PairWait = 4  # Waiting for one of a pair (単騎待ち)

    Name = {
        DoubleEdgeWait: "両面待ち",
        EdgeWait: "辺張待ち",
        ClosedWait: "嵌張待ち",
        TripletWait: "双ポン待ち",
        PairWait: "単騎待ち",
    }


class RuleFlag:
    Null = 0
    RedDora = 1  # Allow red dora (赤ドラ有り)
    OpenTanyao = 2  # Allow open Tanyao (喰い断有り)

    Name = {
        RedDora: "赤ドラ有り",
        OpenTanyao: "喰い断有り",
    }


class ShantenFlag:
    Null = 0
    Regular = 1  # 一般形に対する向聴数
    SevenPairs = 2  # 七対子に対する向聴数
    ThirteenOrphans = 4  # 国士無双に対する向聴数
    All = Regular | SevenPairs | ThirteenOrphans

    Name = {
        Regular: "一般形",
        SevenPairs: "七対子",
        ThirteenOrphans: "国士無双",
    }


class WinFlag:
    Null = 0
    Tsumo = 1 << 1  # Tsumo win (自摸和了)
    Riichi = 1 << 2  # Riichi established (立直成立)
    Ippatsu = 1 << 3  # One-shot Win established (一発成立)
    RobbingAKong = 1 << 4  # Robbing a Kong established (搶槓成立)
    AfterAKong = 1 << 5  # After a Kong established (嶺上開花成立)
    UnderTheSea = 1 << 6  # Under the Sea established (海底撈月成立)
    UnderTheRiver = 1 << 7  # Under the River established (河底撈魚成立)
    DoubleRiichi = 1 << 8  # Double Riichi established (ダブル立直成立)
    NagashiMangan = 1 << 9  # Mangan at Draw established (流し満貫成立)
    BlessingOfHeaven = 1 << 10  # Blessing of Heaven established (天和成立)
    BlessingOfEarth = 1 << 11  # Blessing of Earth established (地和成立)
    HandOfMan = 1 << 12  # Hand of Man established (人和成立)

    Name = {
        Null: "Null",
        Tsumo: "自摸和了",
        Riichi: "立直成立",
        Ippatsu: "一発成立",
        RobbingAKong: "搶槓成立",
        AfterAKong: "嶺上開花成立",
        UnderTheSea: "海底撈月成立",
        UnderTheRiver: "河底撈魚成立",
        DoubleRiichi: "ダブル立直成立",
        NagashiMangan: "流し満貫成立",
        BlessingOfHeaven: "天和成立",
        BlessingOfEarth: "地和成立",
        HandOfMan: "人和成立",
    }


class Yaku:
    Null = 0
    Tsumo = 1  # Win by self-draw (門前清自摸和) */
    Riichi = 1 << 1  # Riichi (立直) */
    Ippatsu = 1 << 2  # One-shot Win (一発) */
    Tanyao = 1 << 3  # All Simples (断幺九) */
    Pinfu = 1 << 4  # Pinfu (平和) */
    PureDoubleSequence = 1 << 5  # Pure Double Sequence (一盃口) */
    RobbingAKong = 1 << 6  # Robbing a kong (槍槓) */
    AfterAKong = 1 << 7  # After a Kong (嶺上開花) */
    UnderTheSea = 1 << 8  # Under the Sea (海底摸月) */
    UnderTheRiver = 1 << 9  # Under the River (河底撈魚) */
    Dora = 1 << 10  # Dora (ドラ) */
    UraDora = 1 << 11  # Ura Dora (裏ドラ) */
    RedDora = 1 << 12  # Red Dora (赤ドラ) */
    WhiteDragon = 1 << 13  # White Dragon (三元牌 白) */
    GreenDragon = 1 << 14  # Green Dragon (三元牌 發) */
    RedDragon = 1 << 15  # Red Dragon (三元牌 中) */
    SelfWindEast = 1 << 16  # Self Wind East (自風 東) */
    SelfWindSouth = 1 << 17  # Self Wind South (自風 南) */
    SelfWindWest = 1 << 18  # Self Wind West (自風 西) */
    SelfWindNorth = 1 << 19  # Self Wind North (自風 北) */
    RoundWindEast = 1 << 20  # Round Wind East (場風 東) */
    RoundWindSouth = 1 << 21  # Round Wind South (場風 南) */
    RoundWindWest = 1 << 22  # Round Wind West (場風 西) */
    RoundWindNorth = 1 << 23  # Round Wind North (場風 北) */
    DoubleRiichi = 1 << 24  # Double Riichi (ダブル立直) */
    SevenPairs = 1 << 25  # Seven Pairs (七対子) */
    AllTriplets = 1 << 26  # All Triplets (対々和) */
    ThreeConcealedTriplets = 1 << 27  # Three Concealed Triplets (三暗刻) */
    TripleTriplets = 1 << 28  # Triple Triplets (三色同刻) */
    MixedTripleSequence = 1 << 29  # Mixed Triple Sequence (三色同順) */
    AllTerminalsAndHonors = 1 << 30  # All Terminals and Honors (混老頭) */
    PureStraight = 1 << 31  # Pure Straight (一気通貫) */
    HalfOutsideHand = 1 << 32  # Half Outside Hand (混全帯幺九) */
    LittleThreeDragons = 1 << 33  # Little Three Dragons (小三元) */
    ThreeKongs = 1 << 34  # Three Quads (三槓子) */
    HalfFlush = 1 << 35  # Half Flush (混一色) */
    FullyOutsideHand = 1 << 36  # Fully Outside Hand (純全帯幺九) */
    TwicePureDoubleSequence = 1 << 37  # Twice Pure Double Sequence (二盃口) */
    NagashiMangan = 1 << 38  # Mangan at Draw (流し満貫) */
    FullFlush = 1 << 39  # Full Flush (清一色) */
    BlessingOfHeaven = 1 << 40  # Blessing of Heaven (天和) */
    BlessingOfEarth = 1 << 41  # Blessing of Earth (地和) */
    HandOfMan = 1 << 42  # Hand of Man (人和) */
    AllGreen = 1 << 43  # All Green (緑一色) */
    BigThreeDragons = 1 << 44  # Big Three Dragons (大三元) */
    LittleFourWinds = 1 << 45  # Little Four Winds (小四喜) */
    AllHonors = 1 << 46  # All Honors (字一色) */
    ThirteenOrphans = 1 << 47  # Thirteen Orphans (国士無双) */
    NineGates = 1 << 48  # Nine Gates (九連宝燈) */
    FourConcealedTriplets = 1 << 49  # Four Concealed Triplets (四暗刻) */
    AllTerminals = 1 << 50  # All Terminals (清老頭) */
    FourKongs = 1 << 51  # Four Kongs (四槓子) */
    SingleWaitFourConcealedTriplets = 1 << 52
    # Single-wait Four Concealed Triplets (四暗刻単騎) */
    BigFourWinds = 1 << 53  # Big Four Winds (大四喜) */
    TrueNineGates = 1 << 54  # True Nine Gates (純正九連宝燈) */
    ThirteenWaitThirteenOrphans = 1 << 55
    # Thirteen-wait Thirteen Orphans (国士無双13面待ち) */

    Name = {
        Null: "Null",
        Tsumo: "門前清自摸和",
        Riichi: "立直",
        Ippatsu: "一発",
        Tanyao: "断幺九",
        Pinfu: "平和",
        PureDoubleSequence: "一盃口",
        RobbingAKong: "槍槓",
        AfterAKong: "嶺上開花",
        UnderTheSea: "海底摸月",
        UnderTheRiver: "河底撈魚",
        Dora: "ドラ",
        UraDora: "裏ドラ",
        RedDora: "赤ドラ",
        WhiteDragon: "三元牌 白",
        GreenDragon: "三元牌 發",
        RedDragon: "三元牌 中",
        SelfWindEast: "自風 東",
        SelfWindSouth: "自風 南",
        SelfWindWest: "自風 西",
        SelfWindNorth: "自風 北",
        RoundWindEast: "場風 東",
        RoundWindSouth: "場風 南",
        RoundWindWest: "場風 西",
        RoundWindNorth: "場風 北",
        DoubleRiichi: "ダブル立直",
        SevenPairs: "七対子",
        AllTriplets: "対々和",
        ThreeConcealedTriplets: "三暗刻",
        TripleTriplets: "三色同刻",
        MixedTripleSequence: "三色同順",
        AllTerminalsAndHonors: "混老頭",
        PureStraight: "一気通貫",
        HalfOutsideHand: "混全帯幺九",
        LittleThreeDragons: "小三元",
        ThreeKongs: "三槓子",
        HalfFlush: "混一色",
        FullyOutsideHand: "純全帯幺九",
        TwicePureDoubleSequence: "二盃口",
        NagashiMangan: "流し満貫",
        FullFlush: "清一色",
        BlessingOfHeaven: "天和",
        BlessingOfEarth: "地和",
        HandOfMan: "人和",
        AllGreen: "緑一色",
        BigThreeDragons: "大三元",
        LittleFourWinds: "小四喜",
        AllHonors: "字一色",
        ThirteenOrphans: "国士無双",
        NineGates: "九連宝燈",
        FourConcealedTriplets: "四暗刻",
        AllTerminals: "清老頭",
        FourKongs: "四槓子",
        SingleWaitFourConcealedTriplets: "四暗刻単騎",
        BigFourWinds: "大四喜",
        TrueNineGates: "純正九連宝燈",
        ThirteenWaitThirteenOrphans: "国士無双13面待ち",
    }


class ScoreTitle:
    Null = -1
    Mangan = 0  # Mangan (満貫)
    Haneman = 1  # Haneman (跳満)
    Baiman = 2  # Baiman (倍満)
    Sanbaiman = 3  # Sanbaiman (三倍満)
    CountedYakuman = 4  # Counted Yakuman (数え役満)
    Yakuman = 5  # Yakuman (役満)
    DoubleYakuman = 6  # Double Yakuman (ダブル役満)
    TripleYakuman = 7  # Triple Yakuman (トリプル役満)
    QuadrupleYakuman = 8  # Quadruple Yakuman (四倍役満)
    QuintupleYakuman = 9  # Quintuple Yakuman (五倍役満)
    SextupleYakuman = 10  # Sextuple Yakuman (六倍役満)

    Name = {
        Null: "Null",
        Mangan: "満貫",
        Haneman: "跳満",
        Baiman: "倍満",
        Sanbaiman: "三倍満",
        CountedYakuman: "数え役満",
        Yakuman: "役満",
        DoubleYakuman: "ダブル役満",
        TripleYakuman: "トリプル役満",
        QuadrupleYakuman: "四倍役満",
        QuintupleYakuman: "五倍役満",
        SextupleYakuman: "六倍役満",
    }
